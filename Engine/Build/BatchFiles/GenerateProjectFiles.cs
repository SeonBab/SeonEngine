// 소스 폴더를 훑어 .vcxproj의 파일 목록과 .vcxproj.filters를 만든다.
// .vcxproj는 손으로 관리하는 파일이라 ClInclude / ClCompile / None 목록만 바꾸고 나머지는 그대로 둔다.

using System.Security.Cryptography;
using System.Text;
using System.Xml;
using System.Xml.Linq;

string scriptDir = (string)AppContext.GetData("EntryPointFileDirectoryPath")!;
string rootDir = Path.GetFullPath(Path.Combine(scriptDir, "..", "..", ".."));

// 한 프로젝트라도 처리할 수 없으면 아무 파일도 쓰지 않도록, 모든 결과를 만든 뒤에 쓴다.
var outputs = new List<(string ProjectName, string ProjectPath, XDocument Project, XDocument Filters, int SourceCount)>();
try
{
	foreach (string projectPath in FindProjects(rootDir))
	{
		string projectDir = Path.GetDirectoryName(projectPath)!;
		List<string> sources = CollectSources(projectDir);

		XDocument project = XDocument.Load(projectPath, LoadOptions.PreserveWhitespace);
		HashSet<string> compiledShaders = ReadCompiledShaders(project, sources, projectPath);
		ReplaceSourceItems(project, sources, projectPath, compiledShaders);

		outputs.Add((Path.GetFileNameWithoutExtension(projectPath), projectPath, project, MakeFilters(sources, compiledShaders), sources.Count));
	}
}
catch (InvalidOperationException e)
{
	Console.Error.WriteLine("오류: " + e.Message);
	Console.Error.WriteLine("아무 파일도 바꾸지 않았다.");
	return 1;
}

foreach (var output in outputs)
{
	bool projectChanged = WriteIfChanged(output.ProjectPath, output.Project, indent: false);
	bool filtersChanged = WriteIfChanged(output.ProjectPath + ".filters", output.Filters, indent: true);

	Console.WriteLine($"{output.ProjectName}: 소스 {output.SourceCount}개, .vcxproj {ToStatus(projectChanged)}, .filters {ToStatus(filtersChanged)}");
}

return 0;

static string ToStatus(bool changed)
{
	return changed ? "갱신" : "변경 없음";
}

// 저장소 안의 .vcxproj 경로를 모은다. 빌드 결과와 서드파티 폴더는 제외한다.
static List<string> FindProjects(string rootDir)
{
	var excludedDirs = new HashSet<string>(StringComparer.OrdinalIgnoreCase) { "Binaries", "Intermediate", "ThirdParty" };

	var projects = new List<string>();
	foreach (string path in Directory.EnumerateFiles(rootDir, "*.vcxproj", SearchOption.AllDirectories))
	{
		// 이름 일부가 겹치는 폴더(BinariesUtil 등)까지 걸러지지 않도록 경로 조각 단위로 비교한다.
		string[] segments = Path.GetRelativePath(rootDir, path).Split(Path.DirectorySeparatorChar);
		if (segments.Any(excludedDirs.Contains))
		{
			continue;
		}

		projects.Add(path);
	}

	// 실행할 때마다 같은 결과가 나오도록 문화권과 무관한 서수 비교로 정렬한다.
	projects.Sort(StringComparer.OrdinalIgnoreCase);
	return projects;
}

// 프로젝트 폴더 아래의 소스 파일을 프로젝트 폴더 기준 상대 경로로 모은다.
// 프로젝트가 Source 폴더에 있으면 옆의 Shaders 폴더도 모은다(Engine/Source -> Engine/Shaders).
// Unreal도 셰이더를 Source 밖에 두고, 프로젝트 파일을 만들 때 편집용으로 넣는다(ProjectFileGenerator.cs의 AddEngineShaderSource).
static List<string> CollectSources(string projectDir)
{
	var searchDirs = new List<string> { projectDir };
	if (string.Equals(Path.GetFileName(projectDir), "Source", StringComparison.OrdinalIgnoreCase))
	{
		string shadersDir = Path.Combine(Path.GetDirectoryName(projectDir)!, "Shaders");
		if (Directory.Exists(shadersDir))
		{
			searchDirs.Add(shadersDir);
		}
	}

	var sources = new List<string>();
	foreach (string searchDir in searchDirs)
	{
		foreach (string path in Directory.EnumerateFiles(searchDir, "*", SearchOption.AllDirectories))
		{
			if (GetItemType(path) != null)
			{
				sources.Add(Path.GetRelativePath(projectDir, path));
			}
		}
	}

	sources.Sort(StringComparer.OrdinalIgnoreCase);
	return sources;
}

// 솔루션 탐색기에 보일 필터 이름. 프로젝트 폴더 밖의 파일(..\Shaders\...)은 앞의 ..\를 떼어 Shaders\...로 보이게 한다.
// null이면 필터 없이 프로젝트 바로 아래에 보인다.
static string? GetFilter(string source)
{
	string? dir = Path.GetDirectoryName(source);
	string parentPrefix = ".." + Path.DirectorySeparatorChar;
	while (dir != null && dir.StartsWith(parentPrefix, StringComparison.Ordinal))
	{
		dir = dir[parentPrefix.Length..];
	}

	return string.IsNullOrEmpty(dir) ? null : dir;
}

// 확장자에 맞는 MSBuild 항목 종류. null이면 프로젝트에 넣지 않는 파일이다.
// 검색 패턴에는 Windows 호환 규칙이 섞여 있으므로 확장자는 여기서 직접 비교한다.
static string? GetItemType(string path)
{
	return Path.GetExtension(path).ToLowerInvariant() switch
	{
		".h" => "ClInclude",
		".cpp" => "ClCompile",
		// 명시적인 FxCompile 등록이 없는 셰이더는 편집용으로만 보인다.
		".hlsl" or ".hlsli" => "None",
		_ => null,
	};
}

// 컴파일 설정은 .vcxproj에서 관리한다. 명시적으로 등록된 HLSL만 읽어 None 중복을 막고 필터 종류를 맞춘다.
// FxCompile 전용 ItemGroup과 그 메타데이터는 다시 쓰지 않는다.
static HashSet<string> ReadCompiledShaders(XDocument project, List<string> sources, string projectPath)
{
	XNamespace ns = project.Root!.Name.Namespace;
	string projectDir = Path.GetDirectoryName(projectPath)!;
	var compiledShaders = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
	var knownSources = new HashSet<string>(sources, StringComparer.OrdinalIgnoreCase);
	foreach (XElement item in project.Root.Elements(ns + "ItemGroup").Elements(ns + "FxCompile"))
	{
		string? include = (string?)item.Attribute("Include");
		if (string.IsNullOrWhiteSpace(include) || include.IndexOfAny(['$', '*', '?', ';', '%']) >= 0)
		{
			throw new InvalidOperationException($"{projectPath}: FxCompile은 명시적인 단일 HLSL 파일 경로가 필요하다.");
		}

		string source = Path.GetRelativePath(projectDir, Path.GetFullPath(Path.Combine(projectDir, include)));
		if (!string.Equals(Path.GetExtension(source), ".hlsl", StringComparison.OrdinalIgnoreCase) || !knownSources.Contains(source))
		{
			throw new InvalidOperationException($"{projectPath}: FxCompile 파일이 검색 범위의 HLSL이 아니다: {include}");
		}
		if (!compiledShaders.Add(source))
		{
			throw new InvalidOperationException($"{projectPath}: FxCompile 파일이 중복 등록되어 있다: {include}");
		}
	}
	return compiledShaders;
}

// .vcxproj에서 ClInclude / ClCompile / None만 담은 ItemGroup을 지우고, 첫 그룹이 있던 자리에 새 목록을 넣는다.
// 그룹이 없으면 Microsoft.Cpp.targets import 앞에 넣는다.
static void ReplaceSourceItems(XDocument project, List<string> sources, string projectPath, HashSet<string> compiledShaders)
{
	XNamespace ns = project.Root!.Name.Namespace;
	string[] sourceItemTypes = ["ClInclude", "ClCompile", "None"];

	var oldGroups = new List<XElement>();
	foreach (XElement group in project.Root.Elements(ns + "ItemGroup"))
	{
		if (!group.Elements().Any(item => sourceItemTypes.Contains(item.Name.LocalName)))
		{
			continue;
		}

		// 파일별 설정(메타데이터)이나 조건이 있는 목록은 다시 만들면 설정이 사라지므로 처리하지 않는다.
		bool onlyPlainSourceItems = !group.HasAttributes && group.Elements().All(item =>
			sourceItemTypes.Contains(item.Name.LocalName) &&
			!item.HasElements &&
			item.Attributes().All(attribute => attribute.Name == "Include"));
		if (!onlyPlainSourceItems)
		{
			throw new InvalidOperationException($"{projectPath}: 소스 목록 ItemGroup에 파일별 설정, 조건, 다른 항목이 있다. 이 스크립트는 Include만 있는 항목을 다룬다.");
		}

		oldGroups.Add(group);
	}

	// 기존 파일의 들여쓰기(2칸)에 맞춰 공백 노드를 직접 넣는다. PreserveWhitespace로 읽은 문서라 자동 들여쓰기를 쓰지 않는다.
	var newGroups = new List<XElement>();
	foreach (string itemType in sourceItemTypes)
	{
		var group = new XElement(ns + "ItemGroup");
		foreach (string source in sources.Where(source => GetItemType(source) == itemType && !compiledShaders.Contains(source)))
		{
			group.Add(new XText("\n    "), new XElement(ns + itemType, new XAttribute("Include", source)));
		}

		if (group.HasElements)
		{
			group.Add(new XText("\n  "));
			newGroups.Add(group);
		}
	}

	XElement anchor;
	if (oldGroups.Count > 0)
	{
		anchor = oldGroups[0];
	}
	else
	{
		anchor = project.Root.Elements(ns + "Import").FirstOrDefault(import => ((string?)import.Attribute("Project"))?.EndsWith(@"\Microsoft.Cpp.targets", StringComparison.OrdinalIgnoreCase) == true)
			?? throw new InvalidOperationException($"{projectPath}: 소스 목록을 넣을 위치(Microsoft.Cpp.targets import)를 찾지 못했다.");
	}

	foreach (XElement group in newGroups)
	{
		anchor.AddBeforeSelf(group, new XText("\n  "));
	}

	foreach (XElement group in oldGroups)
	{
		// 그룹 앞의 줄바꿈과 들여쓰기도 함께 지워 빈 줄이 남지 않게 한다.
		if (group.PreviousNode is XText whitespace && string.IsNullOrWhiteSpace(whitespace.Value))
		{
			whitespace.Remove();
		}
		group.Remove();
	}
}

// .vcxproj.filters 내용을 만든다. 배치는 VS가 저장하는 모양(필터 / ClInclude / ClCompile / None / FxCompile을 각각 다른 ItemGroup에)을 따른다.
static XDocument MakeFilters(List<string> sources, HashSet<string> compiledShaders)
{
	XNamespace ns = "http://schemas.microsoft.com/developer/msbuild/2003";
	var project = new XElement(ns + "Project", new XAttribute("ToolsVersion", "4.0"));

	// 파일이 있는 폴더와 그 상위 폴더를 모두 필터로 정의한다. VS와 Unreal 모두 단계마다 따로 정의한다.
	var filters = new SortedSet<string>(StringComparer.OrdinalIgnoreCase);
	foreach (string source in sources)
	{
		for (string? dir = GetFilter(source); !string.IsNullOrEmpty(dir); dir = Path.GetDirectoryName(dir))
		{
			filters.Add(dir);
		}
	}

	if (filters.Count > 0)
	{
		project.Add(new XElement(ns + "ItemGroup",
			filters.Select(filter => new XElement(ns + "Filter",
				new XAttribute("Include", filter),
				new XElement(ns + "UniqueIdentifier", MakeFilterGuid(filter).ToString("B").ToUpperInvariant())))));
	}

	foreach (string itemType in new[] { "ClInclude", "ClCompile", "None", "FxCompile" })
	{
		var items = new List<XElement>();
		foreach (string source in sources.Where(source => (compiledShaders.Contains(source) ? "FxCompile" : GetItemType(source)) == itemType))
		{
			var item = new XElement(ns + itemType, new XAttribute("Include", source));

			// 프로젝트 폴더 바로 아래의 파일은 필터 없이 프로젝트 아래에 보인다.
			string? dir = GetFilter(source);
			if (dir != null)
			{
				item.Add(new XElement(ns + "Filter", dir));
			}

			items.Add(item);
		}

		if (items.Count > 0)
		{
			project.Add(new XElement(ns + "ItemGroup", items));
		}
	}

	return new XDocument(project);
}

// 필터 경로로 GUID를 계산한다. 실행할 때마다 새 GUID를 만들면 내용이 같아도 diff가 생긴다.
// Unreal과 같은 이름 기반 UUID 버전 3(MD5, RFC 4122)이다.
static Guid MakeFilterGuid(string filter)
{
	// 이 스크립트 전용 네임스페이스. 바꾸면 모든 필터의 GUID가 바뀐다.
	var guidNamespace = new Guid("d9a9f5f5-bba9-4154-84c6-0a9896130fe3");

	byte[] hash = MD5.HashData([.. guidNamespace.ToByteArray(bigEndian: true), .. Encoding.UTF8.GetBytes(filter)]);
	hash[6] = (byte)((hash[6] & 0x0F) | 0x30); // 버전 3
	hash[8] = (byte)((hash[8] & 0x3F) | 0x80); // RFC 4122 변형
	return new Guid(hash, bigEndian: true);
}

// 저장소 규칙(BOM 없는 UTF-8, CRLF, 파일 끝 줄바꿈)대로 쓴다. 내용이 같으면 쓰지 않는다.
// 파일을 다시 쓰기만 해도 열려 있는 VS가 프로젝트를 다시 로드하라고 묻는다.
// indent: 새로 만든 문서는 true. 공백을 보존해 읽은 문서는 원래 들여쓰기를 지키도록 false.
static bool WriteIfChanged(string path, XDocument document, bool indent)
{
	// XML 파서는 CRLF를 LF로 바꿔 읽으므로, 쓸 때 줄바꿈을 CRLF로 되돌린다.
	var settings = new XmlWriterSettings
	{
		Encoding = new UTF8Encoding(false),
		Indent = indent,
		IndentChars = "  ",
		NewLineChars = "\r\n",
		NewLineHandling = NewLineHandling.Replace,
	};

	// StringWriter에 쓰면 XML 선언이 encoding="utf-16"이 되므로 바이트 스트림에 쓴다.
	using var stream = new MemoryStream();
	using (var writer = XmlWriter.Create(stream, settings))
	{
		document.Save(writer);
	}

	// 새로 만든 문서는 파일 끝 줄바꿈이 없다. 읽은 문서는 원래 줄바꿈이 남아 있다.
	if (!stream.ToArray().AsSpan().EndsWith("\r\n"u8))
	{
		stream.Write("\r\n"u8);
	}

	byte[] content = stream.ToArray();
	if (File.Exists(path) && File.ReadAllBytes(path).AsSpan().SequenceEqual(content))
	{
		return false;
	}

	File.WriteAllBytes(path, content);
	return true;
}
