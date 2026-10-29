// 명시한 헤더 또는 헤더 목록을 Clang AST로 분석해 제품 리플렉션 기반용 C++를 만든다.
// 일반 빌드와는 독립적인 수동 도구다. .NET 10 SDK와 Clang/MSVC 분석 환경이 필요하다.
using System.Diagnostics;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;

try
{
	Options options = ReadOptions(args);
	string scriptDir = (string)AppContext.GetData("EntryPointFileDirectoryPath")!;
	string rootDir = Path.GetFullPath(Path.Combine(scriptDir, "..", "..", ".."));
	string intermediate = Path.Combine(rootDir, "Intermediate") + Path.DirectorySeparatorChar;
	if (!options.Output.StartsWith(intermediate, StringComparison.OrdinalIgnoreCase))
	{
		throw new InvalidOperationException("출력은 저장소 Intermediate 아래의 별도 폴더여야 합니다.");
	}
	if (!File.Exists(options.Clang))
	{
		throw new InvalidOperationException("clang-cl 실행 파일의 경로를 확인하세요.");
	}

	string[] compilerArgs = ReadStringArray(options.CompilerArgs);
	ValidateCompilerArgs(compilerArgs);
	HeaderInput[] inputs = ReadHeaderInputs(options);

	Directory.CreateDirectory(options.Output);
	// 모든 후보의 임시 generated.h를 먼저 제공한다. 부모 생성물이 없는 첫 실행도 목록 순서에 의존하지 않는다.
	// 기존 생성 파일보다 이 폴더를 먼저 검색한다. 이 폴더는 이번 실행만 소유한다.
	string staging = Path.Combine(options.Output, ".analysis-" + Guid.NewGuid().ToString("N"));
	Directory.CreateDirectory(staging);
	string[] stubs = inputs.Select(input => Path.Combine(staging, input.Stem + ".generated.h")).Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
	try
	{
		foreach (string stub in stubs)
		{
			File.WriteAllText(stub, "#pragma once\n#include \"Reflection/ReflectionMacros.h\"\n", new UTF8Encoding(false));
		}
		var selected = new List<(HeaderInput Input, ReflectedClass Type)>();
		foreach (HeaderInput input in inputs)
		{
			string ast = await Analyze(input.Path, options, rootDir, staging, compilerArgs);
			using JsonDocument document = JsonDocument.Parse(ast, new JsonDocumentOptions { MaxDepth = 128 });
			if (GetString(document.RootElement, "kind") != "TranslationUnitDecl") { throw new InvalidOperationException("Clang AST 루트 형식을 확인하세요."); }
			// 문자열 검사는 후보를 줄이는 용도다. 실제 참여 여부는 현재 구성의 AST 표시로 결정한다.
			if (options.IsBatch && !Walk(document.RootElement).Any(node => IsMarker(node, "UCLASS", input.Path, input.Source)))
			{
				continue;
			}
			selected.Add((input, ReadClass(document.RootElement, input.Path, input.Source)));
		}
		ValidateBatchSelection(selected, options.IsBatch);
		var outputs = new List<(string Name, string Text)>();
		foreach (var item in selected)
		{
			string relativeHeader = Path.GetRelativePath(rootDir, item.Input.Path).Replace('\\', '/');
			string fileId = "SE_REFLECTION_" + Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(relativeHeader)))[..16];
			outputs.AddRange(Generate(item.Input.Stem, fileId, item.Type));
		}
		// 전체 분석·검사·문자열 생성이 끝난 뒤에만 갱신한다. I/O 실패의 다중 파일 롤백은 제공하지 않는다.
		foreach (var output in outputs) { WriteIfChanged(Path.Combine(options.Output, output.Name), output.Text); }
		if (options.IsBatch)
		{
			string[] sources = selected.Select(item => Path.Combine(options.Output, item.Input.Stem + ".generated.cpp"))
				.Order(StringComparer.OrdinalIgnoreCase).ToArray();
			// 성공한 현재 대상만 전달한다. 이전 실행의 남은 파일을 폴더 glob으로 컴파일하지 않는다.
			WriteIfChanged(Path.Combine(options.Output, "GeneratedSources.txt"), sources.Length == 0 ? "" : string.Join("\n", sources) + "\n");
		}
		foreach (var item in selected)
		{
			Console.WriteLine($"{item.Type.Name}: 프로퍼티 {item.Type.Properties.Count}개 → {options.Output}");
		}
		if (options.IsBatch) { Console.WriteLine($"헤더 목록 처리 완료: 참여 클래스 {selected.Count}개"); }
	}
	finally
	{
		foreach (string stub in stubs) { File.Delete(stub); }
		Directory.Delete(staging);
	}

	return 0;
}
catch (Exception e) when (e is IOException or UnauthorizedAccessException or InvalidOperationException or JsonException or ArgumentException or System.ComponentModel.Win32Exception)
{
	Console.Error.WriteLine("GenerateReflection 오류: " + e.Message);

	return 1;
}

static Options ReadOptions(string[] arguments)
{
	var values = new Dictionary<string, string>(StringComparer.Ordinal);
	string[] names = ["--header", "--headers", "--output", "--clang", "--compiler-args"];
	for (int i = 0; i < arguments.Length; i += 2)
	{
		if (i + 1 >= arguments.Length || !names.Contains(arguments[i]) || !values.TryAdd(arguments[i], arguments[i + 1]))
		{
			throw new InvalidOperationException("사용법: --header 파일.h 또는 --headers 목록.json, --output Intermediate/하위폴더 --clang clang-cl.exe --compiler-args 인수.json");
		}
	}
	if (values.Count != 4 || !values.ContainsKey("--output") || !values.ContainsKey("--clang") || !values.ContainsKey("--compiler-args") ||
		values.ContainsKey("--header") == values.ContainsKey("--headers"))
	{
		throw new InvalidOperationException("--header/--headers 중 하나와 --output, --clang, --compiler-args가 필요합니다.");
	}
	bool isBatch = values.ContainsKey("--headers");
	string[] headers;
	if (isBatch)
	{
		string listPath = Path.GetFullPath(values["--headers"]);
		headers = ReadStringArray(listPath).Select(path => Path.GetFullPath(path, Path.GetDirectoryName(listPath)!)).ToArray();
	}
	else
	{
		headers = [Path.GetFullPath(values["--header"])];
	}
	if (headers.Distinct(StringComparer.OrdinalIgnoreCase).Count() != headers.Length) { throw new InvalidOperationException("헤더 목록에 중복 경로가 있습니다."); }

	return new Options(headers, isBatch, Path.GetFullPath(values["--output"]),
		Path.GetFullPath(values["--clang"]), Path.GetFullPath(values["--compiler-args"]));
}

static string[] ReadStringArray(string path)
{
	using JsonDocument document = JsonDocument.Parse(File.ReadAllText(path));
	if (document.RootElement.ValueKind != JsonValueKind.Array) { throw new InvalidOperationException("JSON 문자열 배열이 필요합니다: " + path); }

	return document.RootElement.EnumerateArray().Select(value =>
	{
		if (value.ValueKind != JsonValueKind.String || string.IsNullOrEmpty(value.GetString()))
		{
			throw new InvalidOperationException("배열 항목은 비어 있지 않은 문자열이어야 합니다: " + path);
		}

		return value.GetString()!;
	}).ToArray();
}

static HeaderInput[] ReadHeaderInputs(Options options)
{
	var inputs = new List<HeaderInput>();
	foreach (string header in options.Headers.Order(StringComparer.OrdinalIgnoreCase))
	{
		if (!File.Exists(header) || !Path.GetExtension(header).Equals(".h", StringComparison.OrdinalIgnoreCase))
		{
			throw new InvalidOperationException("존재하는 .h 헤더가 필요합니다: " + header);
		}
		byte[] source = File.ReadAllBytes(header);
		string text = new UTF8Encoding(false, true).GetString(source);
		if (options.IsBatch && !text.Contains("UCLASS", StringComparison.Ordinal)) { continue; }
		ValidateHeaderSource(header, source);
		inputs.Add(new HeaderInput(header, Path.GetFileNameWithoutExtension(header), source));
	}

	return inputs.ToArray();
}

static void ValidateBatchSelection(List<(HeaderInput Input, ReflectedClass Type)> selected, bool isBatch)
{
	if (selected.Select(item => item.Input.Stem).Distinct(StringComparer.OrdinalIgnoreCase).Count() != selected.Count)
	{
		throw new InvalidOperationException("출력 이름이 충돌합니다. 참여 헤더의 파일 이름은 서로 달라야 합니다.");
	}
	var names = new HashSet<string>(StringComparer.Ordinal);
	foreach (var item in selected)
	{
		if (!names.Add(item.Type.Name)) { throw new InvalidOperationException("참여 클래스 이름이 중복됩니다: " + item.Type.Name); }
	}
	if (!isBatch) { return; }
	foreach (var item in selected)
	{
		if (item.Type.BaseName != "UObject" && !names.Contains(item.Type.BaseName))
		{
			throw new InvalidOperationException("부모 헤더도 목록에 포함해야 합니다: " + item.Type.BaseName);
		}
	}
}

static void WriteIfChanged(string path, string text)
{
	byte[] bytes = Encoding.UTF8.GetBytes(text.Replace("\r\n", "\n").Replace("\n", "\r\n"));
	if (!File.Exists(path) || !File.ReadAllBytes(path).AsSpan().SequenceEqual(bytes)) { File.WriteAllBytes(path, bytes); }
}

static void ValidateCompilerArgs(string[] arguments)
{
	// 분석 방식/출력 경로를 덮는 임의 옵션 대신 include·일반 매크로·CRT 구성만 전달한다.
	foreach (string argument in arguments)
	{
		if (string.IsNullOrEmpty(argument) ||
			!(argument.StartsWith("/I", StringComparison.Ordinal) || argument.StartsWith("/external:I", StringComparison.Ordinal) ||
			  argument.StartsWith("/D", StringComparison.Ordinal) || argument is "/MD" or "/MDd") ||
			argument.Contains("SE_REFLECTION_", StringComparison.Ordinal) || argument.Contains("UPROPERTY", StringComparison.Ordinal) ||
			argument.Contains("UCLASS", StringComparison.Ordinal) || argument.Contains("GENERATED_BODY", StringComparison.Ordinal))
		{
			throw new InvalidOperationException("지원하지 않는 컴파일 인수: " + argument);
		}
	}
}

static async Task<string> Analyze(string header, Options options, string rootDir, string staging, string[] compilerArgs)
{
	var start = new ProcessStartInfo(options.Clang)
	{
		UseShellExecute = false,
		RedirectStandardOutput = true,
		RedirectStandardError = true,
		CreateNoWindow = true
	};
	// 문자열로 셸 명령을 합치지 않는다. 공백이 있는 경로도 개별 인수로 전달한다.
	start.ArgumentList.Add("/I" + staging);
	foreach (string argument in compilerArgs) { start.ArgumentList.Add(argument); }
	string[] analysisArgs = ["/nologo", "/std:c++20", "/utf-8", "/EHsc", "/GR", "/W4", "/WX", "/TP", "-fsyntax-only",
		"-Wno-pragma-once-outside-header", "-Wno-unused-private-field",
		"-Xclang", "-ast-dump=json", "/DSE_REFLECTION_ANALYSIS=1", "/I" + Path.Combine(rootDir, "Engine", "Source"),
		header];
	foreach (string argument in analysisArgs) { start.ArgumentList.Add(argument); }
	using Process process = Process.Start(start) ?? throw new InvalidOperationException("Clang을 시작할 수 없습니다.");
	Task<string> output = process.StandardOutput.ReadToEndAsync();
	Task<string> errors = process.StandardError.ReadToEndAsync();
	await process.WaitForExitAsync();
	string diagnostics = await errors;
	if (diagnostics.Length != 0) { Console.Error.Write(diagnostics); }
	if (process.ExitCode != 0) { throw new InvalidOperationException($"Clang 분석 실패: 종료 {process.ExitCode}"); }

	return await output;
}

static ReflectedClass ReadClass(JsonElement root, string header, byte[] source)
{
	if (GetString(root, "kind") != "TranslationUnitDecl") { throw new InvalidOperationException("Clang AST 루트 형식을 확인하세요."); }
	JsonElement[] declarations = Children(root).ToArray();
	JsonElement[] classMarkers = Walk(root).Where(n => IsMarker(n, "UCLASS", header, source)).ToArray();
	if (classMarkers.Length != 1) { throw new InvalidOperationException("헤더에 전역 UCLASS 클래스 하나가 필요합니다."); }
	int markerIndex = Array.FindIndex(declarations, n => n.Equals(classMarkers[0]));
	if (markerIndex < 0 || markerIndex + 1 >= declarations.Length)
	{
		throw new InvalidOperationException("UCLASS는 전역 클래스 정의 바로 앞에 있어야 합니다. namespace/중첩/템플릿은 지원하지 않습니다.");
	}
	JsonElement type = declarations[markerIndex + 1];
	if (GetString(type, "kind") != "CXXRecordDecl" || !GetBool(type, "completeDefinition") || GetString(type, "tagUsed") != "class")
	{
		throw new InvalidOperationException("UCLASS 뒤에는 class 정의 하나가 필요합니다.");
	}
	string name = GetString(type, "name");
	RequireIdentifier(name);
	JsonElement[] bases = type.TryGetProperty("bases", out var baseArray) ? baseArray.EnumerateArray().ToArray() : [];
	if (bases.Length != 1 || GetString(bases[0], "access") != "public" || GetBool(bases[0], "isVirtual"))
	{
		throw new InvalidOperationException("public 비가상 단일 상속만 지원합니다.");
	}
	string baseName = GetString(bases[0].GetProperty("type"), "qualType");
	RequireIdentifier(baseName);
	if (baseName != "UObject")
	{
		ValidateReflectedBase(root, declarations, baseName);
	}

	JsonElement[] members = Children(type).ToArray();
	JsonElement[] bodies = members.Where(n => IsMarker(n, "GENERATED_BODY", header, source)).ToArray();
	if (bodies.Length != 1 || Walk(root).Count(n => IsMarker(n, "GENERATED_BODY", header, source)) != 1)
	{
		throw new InvalidOperationException("참여 클래스 안에 GENERATED_BODY() 하나가 필요합니다.");
	}
	int bodyOffset = MarkerOffset(bodies[0]);
	int bodyLine = source.AsSpan(0, bodyOffset).Count((byte)'\n') + 1;
	Dictionary<string, string> sourceFiles = ReadSourceFiles(root);
	var properties = new List<ReflectedProperty>();
	foreach (JsonElement member in members)
	{
		string memberName = GetString(member, "name");
		if (memberName is "StaticTypeInfo" or "GetTypeInfo" || memberName.StartsWith("SEGetPropertyValue_", StringComparison.Ordinal) || memberName.StartsWith("SESetPropertyValue_", StringComparison.Ordinal))
		{
			throw new InvalidOperationException("생성 코드와 충돌하는 멤버 이름: " + memberName);
		}
		if (!Children(member).Any(n => IsMarker(n, "UPROPERTY", header, source))) { continue; }
		if (GetString(member, "kind") != "FieldDecl" || GetBool(member, "isBitfield") || GetBool(member, "mutable"))
		{
			throw new InvalidOperationException("UPROPERTY는 지원하는 값 타입의 일반 인스턴스 필드만 지원합니다: " + memberName);
		}
		RequireIdentifier(memberName);
		properties.Add(new ReflectedProperty(memberName, ReadPropertyType(root, type, member, sourceFiles)));
	}
	// static 필드·메서드·중첩 클래스 등에 붙여 무시된 것처럼 보이는 표시도 거부한다.
	if (Walk(root).Count(n => IsMarker(n, "UPROPERTY", header, source)) != properties.Count)
	{
		throw new InvalidOperationException("참여 클래스의 지원하는 일반 필드 이외에 UPROPERTY를 사용할 수 없습니다.");
	}

	return new ReflectedClass(name, baseName, bodyLine, properties);
}

static string ReadPropertyType(JsonElement root, JsonElement owner, JsonElement member, IReadOnlyDictionary<string, string> sourceFiles)
{
	JsonElement type = member.GetProperty("type");
	string qualified = GetString(type, "qualType");
	string canonical = GetString(type, "desugaredQualType");
	if (canonical.Length == 0) { canonical = qualified; }
	// 엔진 별칭 한 종류만 명시적으로 지원한다. 이름이 같은 사용자 별칭이나 자산 클래스를 허용하지 않는다.
	if (!type.TryGetProperty("typeAliasDeclId", out _) && qualified == "TSharedPtr<UStaticMesh>" && canonical == "std::shared_ptr<UStaticMesh>" &&
		!HasTypeNameShadow(root, owner, "TSharedPtr") && !HasTypeNameShadow(root, owner, "UStaticMesh") &&
		IsEngineDeclaration(root, sourceFiles, "TypeAliasTemplateDecl", "TSharedPtr", "Core/Templates/SharedPtr.h") &&
		IsEngineDeclaration(root, sourceFiles, "CXXRecordDecl", "UStaticMesh", "Renderer/StaticMesh.h"))
	{
		return qualified;
	}
	// 표기 이름만 같거나 별칭으로 다른 타입을 숨기는 경우를 허용하지 않는다.
	if (!type.TryGetProperty("typeAliasDeclId", out _) && qualified == canonical)
	{
		if (qualified == "float") { return "float"; }
		if ((qualified is "FVector3" or "FRotator") && !HasTypeNameShadow(root, owner, qualified))
		{
			string scriptDir = (string)AppContext.GetData("EntryPointFileDirectoryPath")!;
			string fileName = qualified == "FVector3" ? "Vector3.h" : "Rotator.h";
			string mathHeader = Path.GetFullPath(Path.Combine(scriptDir, "..", "..", "Source", "Core", "Math", fileName));
			// 엔진의 실제 전역 정의를 확인한다. 다른 파일의 동명 구조체나 중첩 타입을 수학 값 타입으로 생성하지 않는다.
			foreach (JsonElement declaration in Children(root))
			{
				if (GetString(declaration, "kind") == "CXXRecordDecl" && GetString(declaration, "name") == qualified &&
					GetBool(declaration, "completeDefinition") && declaration.TryGetProperty("loc", out var location) &&
					location.TryGetProperty("file", out var file) && Path.GetFullPath(file.GetString()!).Equals(mathHeader, StringComparison.OrdinalIgnoreCase))
				{
					return qualified;
				}
			}
		}
	}
	throw new InvalidOperationException("UPROPERTY는 float, 엔진 FVector3/FRotator 또는 TSharedPtr<UStaticMesh> 필드만 지원합니다: " + GetString(member, "name"));
}

static Dictionary<string, string> ReadSourceFiles(JsonElement root)
{
	var files = new Dictionary<string, string>(StringComparer.Ordinal);
	string currentFile = "";
	Visit(root);
	return files;

	void Visit(JsonElement element)
	{
		if (element.ValueKind == JsonValueKind.Object)
		{
			// Clang JSON은 같은 실제 파일을 반복 출력하지 않는다. 포함 경로/presumedFile은 이 상태를 바꾸지 않는다.
			if (element.TryGetProperty("offset", out _) && element.TryGetProperty("col", out _) && element.TryGetProperty("tokLen", out _))
			{
				if (element.TryGetProperty("file", out var file)) { currentFile = file.GetString() ?? ""; }
			}
			// inner 배열만 순회하면 loc/range의 파일 전환을 놓친다. JSON 출력 순서대로 모든 값을 읽는다.
			foreach (JsonProperty property in element.EnumerateObject())
			{
				Visit(property.Value);
				// 선언의 직접 위치만 식별자에 연결한다. 매크로 위치를 추측해 실제 정의로 인정하지 않는다.
				if (property.Name == "loc" && property.Value.TryGetProperty("offset", out _) && currentFile.Length != 0)
				{
					string id = GetString(element, "id");
					if (id.Length != 0) { files[id] = currentFile; }
				}
			}
		}
		else if (element.ValueKind == JsonValueKind.Array)
		{
			foreach (JsonElement child in element.EnumerateArray()) { Visit(child); }
		}
	}
}

static bool IsEngineDeclaration(JsonElement root, IReadOnlyDictionary<string, string> sourceFiles, string kind, string name, string header)
{
	string scriptDir = (string)AppContext.GetData("EntryPointFileDirectoryPath")!;
	string expectedFile = Path.GetFullPath(Path.Combine(scriptDir, "..", "..", "Source", header));
	foreach (JsonElement declaration in Children(root))
	{
		if (GetString(declaration, "kind") != kind || GetString(declaration, "name") != name) { continue; }
		// 공유 포인터 선언은 불완전 타입으로도 가능하므로 실제 엔진 정의까지 확인한다.
		if (kind == "CXXRecordDecl" && !GetBool(declaration, "completeDefinition")) { continue; }
		if (sourceFiles.TryGetValue(GetString(declaration, "id"), out string? file) &&
			Path.GetFullPath(file).Equals(expectedFile, StringComparison.OrdinalIgnoreCase)) { return true; }
	}
	return false;
}

static bool HasTypeNameShadow(JsonElement root, JsonElement owner, string typeName)
{
	// Clang JSON은 중첩 구조체도 단순 이름으로 표기할 수 있다. 생성된 멤버 함수의 이름 검색도 확인한다.
	for (JsonElement current = owner; ;)
	{
		if (Children(current).Any(member => GetString(member, "name") == typeName)) { return true; }
		if (!current.TryGetProperty("bases", out var bases) || bases.GetArrayLength() == 0) { return false; }
		string name = GetString(bases[0].GetProperty("type"), "qualType");
		current = Children(root).FirstOrDefault(node => GetString(node, "kind") == "CXXRecordDecl" &&
			GetString(node, "name") == name && GetBool(node, "completeDefinition"));
		if (current.ValueKind == JsonValueKind.Undefined) { return true; }
	}
}

static void ValidateReflectedBase(JsonElement root, JsonElement[] declarations, string baseName)
{
	// 상속받은 StaticTypeInfo가 호출된다는 이유만으로 부모의 참여를 인정하면 기반 정보를 잘못 연결한다.
	// 실제 전역 부모 정의 바로 앞의 UCLASS와 그 헤더의 선언 계약을 조상까지 다시 확인한다.
	int index = Array.FindIndex(declarations, n => GetString(n, "kind") == "CXXRecordDecl" &&
		GetString(n, "name") == baseName && GetBool(n, "completeDefinition"));
	if (index <= 0 || !declarations[index - 1].TryGetProperty("range", out var range) ||
		!range.GetProperty("begin").TryGetProperty("expansionLoc", out var location) ||
		!location.TryGetProperty("file", out var file))
	{
		throw new InvalidOperationException("부모는 UCLASS/GENERATED_BODY를 선언한 전역 참여 클래스여야 합니다: " + baseName);
	}
	string parentHeader = Path.GetFullPath(file.GetString()!);
	byte[] parentSource = File.ReadAllBytes(parentHeader);
	ValidateHeaderSource(parentHeader, parentSource);
	if (!IsMarker(declarations[index - 1], "UCLASS", parentHeader, parentSource))
	{
		throw new InvalidOperationException("부모의 UCLASS 선언이 없습니다: " + baseName);
	}
	ReflectedClass parent = ReadClass(root, parentHeader, parentSource);
	if (parent.Name != baseName)
	{
		throw new InvalidOperationException("부모와 참여 선언이 일치하지 않습니다: " + baseName);
	}
}

static void ValidateHeaderSource(string header, byte[] source)
{
	if (!Path.GetExtension(header).Equals(".h", StringComparison.OrdinalIgnoreCase))
	{
		throw new InvalidOperationException("입력은 .h 헤더여야 합니다: " + header);
	}
	RequireIdentifier(Path.GetFileNameWithoutExtension(header));
	string text = new UTF8Encoding(false, true).GetString(source);
	if (Regex.IsMatch(text, @"^\s*#\s*line\b", RegexOptions.Multiline))
	{
		throw new InvalidOperationException("#line으로 줄 번호를 바꾸는 헤더는 지원하지 않습니다: " + header);
	}
}

static bool IsMarker(JsonElement node, string marker, string header, byte[] source)
{
	string kind = GetString(node, "kind");
	if (kind != (marker == "UPROPERTY" ? "AnnotateAttr" : "StaticAssertDecl")) { return false; }
	if (!node.TryGetProperty("range", out var range) || !range.GetProperty("begin").TryGetProperty("expansionLoc", out var location)) { return false; }
	if (location.TryGetProperty("file", out var file))
	{
		if (!Path.GetFullPath(file.GetString()!).Equals(header, StringComparison.OrdinalIgnoreCase)) { return false; }
	}
	else if (location.TryGetProperty("includedFrom", out _))
	{
		// include 위치는 부모 헤더 자체의 경로가 아니다. 원본 파일이 생략된 경우 추측하지 않는다.
		return false;
	}
	int offset = location.GetProperty("offset").GetInt32();
	int length = location.GetProperty("tokLen").GetInt32();
	if (offset < 0 || offset + length > source.Length) { return false; }

	return Encoding.UTF8.GetString(source, offset, length) == marker;
}

static int MarkerOffset(JsonElement node) => node.GetProperty("range").GetProperty("begin").GetProperty("expansionLoc").GetProperty("offset").GetInt32();
static string GetString(JsonElement node, string name) => node.TryGetProperty(name, out var value) ? value.GetString() ?? "" : "";
static bool GetBool(JsonElement node, string name) => node.TryGetProperty(name, out var value) && value.ValueKind == JsonValueKind.True;
static IEnumerable<JsonElement> Children(JsonElement node) => node.TryGetProperty("inner", out var inner) ? inner.EnumerateArray() : [];
static IEnumerable<JsonElement> Walk(JsonElement node)
{
	yield return node;
	foreach (JsonElement child in Children(node))
	{
		foreach (JsonElement item in Walk(child)) { yield return item; }
	}
}

static void RequireIdentifier(string name)
{
	if (!Regex.IsMatch(name, @"^[A-Za-z_][A-Za-z0-9_]*$")) { throw new InvalidOperationException("ASCII 식별자만 지원합니다: " + name); }
}

static (string Name, string Text)[] Generate(string stem, string fileId, ReflectedClass type)
{
	var declarations = new List<string>
	{
		"public:", "static const FTypeInfo& StaticTypeInfo();", "const FTypeInfo& GetTypeInfo() const override;", "private:"
	};
	var definitions = new List<string>
	{
		"// Generated by GenerateReflection. Do not edit.", "#include \"" + stem + ".h\"", "", "#include \"Reflection/PropertyInfo.h\"", "#include \"Reflection/TypeInfo.h\"", ""
	};
	foreach (ReflectedProperty property in type.Properties)
	{
		declarations.Add($"static {property.ValueType} SEGetPropertyValue_{property.Name}(const {type.Name}& object);");
		declarations.Add($"static void SESetPropertyValue_{property.Name}({type.Name}& object, const {property.ValueType}& value);");
		definitions.AddRange([
			$"{property.ValueType} {type.Name}::SEGetPropertyValue_{property.Name}(const {type.Name}& object)", "{", $"\treturn object.{property.Name};", "}", "",
			$"void {type.Name}::SESetPropertyValue_{property.Name}({type.Name}& object, const {property.ValueType}& value)", "{", $"\tobject.{property.Name} = value;", "}", ""]);
	}
	definitions.AddRange([$"const FTypeInfo& {type.Name}::StaticTypeInfo()", "{"]);
	if (type.Properties.Count != 0)
	{
		definitions.AddRange(["\tstatic const FPropertyInfo properties[] =", "\t{"]);
		definitions.AddRange(type.Properties.Select(p => $"\t\tFPropertyInfo::Make<{type.Name}, {p.ValueType}, &{type.Name}::SEGetPropertyValue_{p.Name}, &{type.Name}::SESetPropertyValue_{p.Name}>(\"{p.Name}\"),"));
		definitions.AddRange(["\t};", ""]);
	}
	string propertiesArg = type.Properties.Count == 0 ? "{}" : "properties";
	definitions.AddRange([$"\tstatic const FTypeInfo typeInfo(\"{type.Name}\", &{type.BaseName}::StaticTypeInfo(), {propertiesArg});", "", "\treturn typeInfo;", "}", "",
		$"const FTypeInfo& {type.Name}::GetTypeInfo() const", "{", "\treturn StaticTypeInfo();", "}", ""]);
	string macro = fileId + "_" + type.BodyLine + "_BODY";
	string generatedHeader = "// Generated by GenerateReflection. Do not edit.\n#pragma once\n\n#include \"Reflection/ReflectionMacros.h\"\n\n#if !SE_REFLECTION_ANALYSIS\n" +
		"#undef SE_REFLECTION_FILE_ID\n#define SE_REFLECTION_FILE_ID " + fileId + "\n\n#define " + macro + " \\\n" +
		string.Join(" \\\n", declarations.Select(d => "\t" + d)) + "\n#endif\n";

	return [(stem + ".generated.h", generatedHeader), (stem + ".generated.cpp", string.Join("\n", definitions))];
}

record Options(string[] Headers, bool IsBatch, string Output, string Clang, string CompilerArgs);
record HeaderInput(string Path, string Stem, byte[] Source);
record ReflectedClass(string Name, string BaseName, int BodyLine, List<ReflectedProperty> Properties);
record ReflectedProperty(string Name, string ValueType);
