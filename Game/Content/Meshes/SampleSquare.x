xof 0303txt 0032

Material SampleSquareMaterial
{
	1.0;1.0;1.0;1.0;;
	32.0;
	0.0;0.0;0.0;;
	0.0;0.0;0.0;;
	TextureFilename { "../../Engine/Textures/Checker.png"; }
}

Frame SampleSquareRoot
{
	FrameTransformMatrix
	{
		1.0,0.0,0.0,0.0,
		0.0,1.0,0.0,0.0,
		0.0,0.0,1.0,0.0,
		0.0,0.0,0.0,1.0;;
	}

	Mesh SampleSquare
	{
		4;
		-0.5;-0.5;0.0;,
		-0.5;0.5;0.0;,
		0.5;0.5;0.0;,
		0.5;-0.5;0.0;;
		2;
		3;0,1,2;,
		3;0,2,3;;

		MeshNormals
		{
			4;
			0.0;0.0;-1.0;,
			0.0;0.0;-1.0;,
			0.0;0.0;-1.0;,
			0.0;0.0;-1.0;;
			2;
			3;0,1,2;,
			3;0,2,3;;
		}

		MeshMaterialList
		{
			1;
			2;
			0,0;
			{ SampleSquareMaterial }
		}

		MeshTextureCoords
		{
			4;
			0.0;1.0;,
			0.0;0.0;,
			1.0;0.0;,
			1.0;1.0;;
		}
	}
}
