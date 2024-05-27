#include "ProceduralPlanetGenerator.h"

// Sets default values
AProceduralPlanetGenerator::AProceduralPlanetGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = ProceduralMesh;	

	StepSize = 100;
	bShouldUpdate = false;
	bIsRounded = false;
	bIsCube = true;

	X_Size = 1;
	Y_Size = 1;
	Z_Size = 1;
	
}

// Called when the game starts or when spawned
void AProceduralPlanetGenerator::BeginPlay()
{
	Super::BeginPlay();

}



// Called every frame
void AProceduralPlanetGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProceduralPlanetGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);


	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
		{
			GenerateMesh();
			if (bIsCube)
			{
				Y_Size = Z_Size = X_Size;
			}
			X_Bounds = (X_Size - 1) * StepSize;
			Y_Bounds = (Y_Size - 1) * StepSize;
			Z_Bounds = (Z_Size - 1) * StepSize;
			Roundness = RoundnessScale * 0.5 * StepSize;
		});
	
}

void AProceduralPlanetGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
		{
			GenerateMesh();
		});
}

void AProceduralPlanetGenerator::GenerateMesh()
{
	FScopeLock Lock(&MeshGenCriticalSection);

	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	UV0.Empty();
	VertexColors.Empty();
	Tangents.Empty();

	CreateVertices();

	CreateTriangles();

	// TODO: Set UV0

	/*UV0.Add(FVector2D(0, 0));
	UV0.Add(FVector2D(0, 1));
	UV0.Add(FVector2D(1, 1));
	UV0.Add(FVector2D(1, 0));*/

/*Tangents.Add(FProcMeshTangent(0, 0, 0));
Tangents.Add(FProcMeshTangent(0, 0, 0));
Tangents.Add(FProcMeshTangent(0, 0, 0));
Tangents.Add(FProcMeshTangent(0, 0, 0));*/

	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		if (bShouldUpdate == true)
		{
			ProceduralMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);
		}
	});
}

void AProceduralPlanetGenerator::CreateVertices()
{
	FScopeLock Lock(&MeshGenCriticalSection);

	// Number of vertices not including duplicates.
	int32 VertexCount = ((2 * X_Size * Y_Size) + (2 * Y_Size * Z_Size) + (2 * Z_Size * X_Size)) - ((4 * X_Size) +  (4 * Y_Size) + (4 * Z_Size)) + 8;

	Vertices.SetNum(VertexCount);
	Normals.SetNum(VertexCount);
	int32 v = 0;	

	if (Vertices.Num() == 0) { return; }

	// Height
	for (int z = 0; z < Z_Size; z++)
	{
		// Front
		for (int x = 0; x < X_Size; x++)
		{
			if (v >= VertexCount) { return; }
			SetVertices(v++, x * StepSize, 0, z * StepSize);
		}
		// Right
		for (int y = 1; y < Y_Size; y++)
		{
			if (v >= VertexCount) { return; }
			SetVertices(v++, X_Bounds, y * StepSize, z * StepSize);
		}
		// Back
		for (int x = X_Size - 2; x > 0; x--)
		{
			if (v >= VertexCount) { return; }
			SetVertices(v++, x * StepSize, Y_Bounds, z * StepSize);
		}
		// Left
		for (int y = Y_Size - 1; y > 0; y--)
		{
			if (v >= VertexCount) { return; }
			SetVertices(v++, 0, y * StepSize, z * StepSize);
		}
	}

	int32 Offset = (X_Size - 2) * (Y_Size - 2);

	// Top & Bottom
	for (int y = 1; y < Y_Size - 1; y++)
	{
		for (int x = 1; x < X_Size - 1; x++)
		{
			// Top
			if (v + Offset >= VertexCount) { return; }
			SetVertices(v, x * StepSize, y * StepSize, Z_Bounds);
			//Vertices[v] = FVector(x * StepSize, y * StepSize, Boundary * StepSize);
			// 
			// Bottom
			SetVertices(v + Offset, x * StepSize, y * StepSize, 0);
			//Vertices[v + Offset] = FVector(x * StepSize, y * StepSize, 0);
			v++;
		}
	}
}

void AProceduralPlanetGenerator::SetVertices(int i, int x, int y, int z)
{
	FVector Inner = Vertices[i] = FVector(x, y, z);

	if (!bIsRounded) { return; }
	if (x < Roundness)
	{
		Inner.X = Roundness;
	}
	else if (x > X_Bounds - Roundness)
	{
		Inner.X = X_Bounds - Roundness;
	}
	if (y < Roundness)
	{
		Inner.Y = Roundness;
	}
	else if (y > Y_Bounds - Roundness)
	{
		Inner.Y = Y_Bounds - Roundness;
	}
	if (z < Roundness)
	{
		Inner.Z = Roundness;
	}
	else if (z > Z_Bounds - Roundness)
	{
		Inner.Z = Z_Bounds - Roundness;
	}

	Normals[i] = (Vertices[i] - Inner);
	Normals[i].Normalize();
	Vertices[i] = Inner + Normals[i] * Roundness;
}

void AProceduralPlanetGenerator::CreateTriangles()
{
	FScopeLock Lock(&MeshGenCriticalSection);

	int32 i = 0;
	int32 v = 0;
	int32 Ring = 2 * X_Size + 2 * Y_Size - 4;
	// Number of vertices including duplicates
	int32 VertexCount = 6 * ((2 * (X_Size - 1) * (Y_Size - 1)) + (2 * (Y_Size - 1) * (Z_Size - 1)) + (2 * (Z_Size - 1) * (X_Size - 1)));
	Triangles.SetNum(VertexCount);

	// Side triangles index buffers
	for (int z = 0; z < Z_Size - 1; z++, v++)
	{
		for (int j = 0; j < Ring - 1; j++, v++)
		{
			Triangles[i] = v;
			Triangles[i + 1] = Triangles[i + 4] = v + Ring;
			Triangles[i + 2] = Triangles[i + 3] = v + 1;			
			Triangles[i+5] = v + Ring + 1;
			i += 6;
		}
		Triangles[i] = v;
		Triangles[i + 1] = Triangles[i + 4] = v + Ring;
		Triangles[i + 2] = Triangles[i + 3] = v - Ring + 1;		
		Triangles[i + 5] = v + 1;
		i += 6;
	}

	// TOP FACE V2
	for (int y = 0; y < Y_Size - 2; y++, v++)
	{
		for (int x = 0; x < X_Size - 2; x++, v++)
		{
			Triangles[i] = v - 1;
			Triangles[i + 1] = Triangles[i + 4] = v - (X_Size - 3);
			Triangles[i + 2] = Triangles[i + 3] = v;
			Triangles[i + 5] = v - (X_Size - 2);

			if (y == 0)
			{
				Triangles[i + 1] = Triangles[i + 4] = v - Ring;
				Triangles[i + 5] = v - Ring + 1;
			}
			if (x == 0)
			{

			}
			i += 6;
		}
		v--;
		Triangles[i] = v;
		Triangles[i + 1] = Triangles[i + 4] = v - Ring - ((X_Size - 2) * y) + 1;
		Triangles[i + 2] = Triangles[i + 3] = v - Ring + 3;
		Triangles[i + 5] = v - Ring + 2;
		i += 6;
	}



	// TOP FACE
	// Starting with X coordinate verts on edges 
	/*v = Z_Size * Ring - Ring;
	for (int x = 0; x < X_Size - 1; x++, v++)
	{
		Triangles[i] = v;
		Triangles[i + 1] = Triangles[i + 4] = v + Ring - 1;
		Triangles[i + 2] = Triangles[i + 3] = v + 1;
		Triangles[i + 5] = v + Ring;
		if (x == X_Size - 2)
		{
			Triangles[i + 2] = Triangles[i + 3] = v + 2;
		}		
		i += 6;
	}*/

	//// Skip Corner?
	//v++;
	//// Now Y at X verts on edges,
	//for (int y = 1; y < Y_Size - 1; y++, v++)
	//{
	//	Triangles[i] = Y_Size * Ring + X_Size - 3;
	//	Triangles[i + 1] = Triangles[i + 4] = Z_Size * Ring + ((X_Size - 2) * (Y_Size - 2)) - 1;
	//	Triangles[i + 2] = Triangles[i + 3] = v;
	//	Triangles[i + 5] = v + 1;
	//	if (y == Y_Size - 2)
	//	{
	//		Triangles[i + 1] = Triangles[i + 4] = v + 2;
	//	}
	//	i += 6;
	//}
	//// Skip Corner
	//v++;
	//// Repeat for X at Y
	//for (int x = X_Size - 3, j = 1; x > 0; x--, v++, j++)
	//{
	//	Triangles[i] = Z_Size * Ring + (X_Size - 2) * (Y_Size - 2) - (j + 1);
	//	Triangles[i + 1] = Triangles[i + 4] = v + 1;
	//	Triangles[i + 2] = Triangles[i + 3] = Z_Size * Ring + (X_Size - 2) * (Y_Size - 2) - j;
	//	Triangles[i + 5] = v;
	//	if (x == 1)
	//	{
	//		Triangles[i] = v + 2;
	//	}
	//	i += 6;
	//}
	////Skip Corner
	//v++;
	//// Again for Y at X0
	//for (int y = Resolution - 2; y > 0; y--, v++)
	//{
	//	Triangles[i] = v + 1;
	//	Triangles[i + 1] = Triangles[i + 4] = v;
	//	Triangles[i + 2] = Triangles[i + 3] = v + y + (Resolution - 2) * (y - 2);
	//	Triangles[i + 5] = v + y + (Resolution - 2) * (y - 1);
	//	if (y == 1)
	//	{
	//		Triangles[i] = v - Ring + 1;
	//		Triangles[i + 2] = Triangles[i + 3] = v - Ring + 2;
	//		Triangles[i + 5] = v + 1;
	//	}
	//	i += 6;
	//}
	//// Inner Grid Tris
	//v = Resolution * Ring;
	//for (int y = 0; y < Resolution - 3; y++, v++)
	//{
	//	for (int x = 0; x < Resolution - 3; x++, v++)
	//	{
	//		Triangles[i] = v;
	//		Triangles[i + 1] = Triangles[i + 4] = v + Resolution - 2;
	//		Triangles[i + 2] = Triangles[i + 3] = v + 1;
	//		Triangles[i + 5] = v + Resolution - 1;
	//		i += 6;
	//	}		
	//}

	//
	//// BOTTOM FACE
	//v = 0;
	//// X at Y0 edge
	//for (int x = 0; x < Resolution - 1; x++, v++)
	//{
	//	Triangles[i] = v;		
	//	Triangles[i + 1] = Triangles[i + 4] = v + 1;
	//	Triangles[i + 2] = Triangles[i + 3] = Ring * Resolution + (Resolution - 2) * (Resolution - 2) + v - 1;
	//	Triangles[i + 5] = v + Ring * Resolution + (Resolution - 2) * (Resolution - 2);
	//	if (x == 0)
	//	{
	//		Triangles[i + 2] = Triangles[i + 3] = v + Ring - 1;
	//	}
	//	if (x == Resolution - 2)
	//	{
	//		Triangles[i + 5] = v + 2;
	//	}
	//	i += 6;
	//}

	//// Skip Corner?
	//v++;
	//// Now Y at X verts on edges,
	//for (int y = 1; y < Resolution - 1; y++, v++)
	//{
	//	Triangles[i] = Resolution * Ring + (Resolution - 2) * (Resolution - 2) + Resolution - 3;
	//	Triangles[i + 1] = Triangles[i + 4] = v;
	//	Triangles[i + 2] = Triangles[i + 3] = Resolution * Ring + (Resolution - 2) * (Resolution - 2) + (Resolution - 2) + v * Resolution - 3 ;
	//	Triangles[i + 5] = v + 1;
	//	if (y == Resolution - 2)
	//	{
	//		Triangles[i + 2] = Triangles[i + 3] = v + 2;
	//	}
	//	i += 6;
	//}
	//// Skip Corner
	//v++;
	//// Repeat for X at Y
	//for (int x = Resolution - 2, j = 1; x > 0; x--, v++, j++)
	//{
	//	Triangles[i] = Resolution * Ring + 2 * (Resolution - 2) * (Resolution - 2) - (j + 1);
	//	Triangles[i + 1] = Triangles[i + 4] = Resolution * Ring + 2 * (Resolution - 2) * (Resolution - 2) - j;
	//	Triangles[i + 2] = Triangles[i + 3] = v + 1;
	//	Triangles[i + 5] = v;
	//	if (x == 1)
	//	{
	//		Triangles[i] = v + 2;
	//	}
	//	i += 6;
	//}
	//// Skip Corner
	//v++;
	//// Again for Y at X0
	//for (int y = Resolution - 3; y > 0; y--, v++)
	//{
	//	Triangles[i] = v + 1;
	//	Triangles[i + 1] = Triangles[i + 4] = Ring * Resolution + (Resolution - 2) * (Resolution - 2) + (Resolution - 2) * (y - 1);
	//	Triangles[i + 2] = Triangles[i + 3] = v;
	//	Triangles[i + 5] = Ring * Resolution + (Resolution - 2) * (Resolution - 2) + (Resolution - 2) * (y);
	//	i += 6;
	//}
	//// Inner Grid Tris
	//v = Resolution * Ring + (Resolution - 2) * (Resolution - 2);
	//for (int y = 0; y < Resolution - 3; y++, v++)
	//{
	//	for (int x = 0; x < Resolution - 3; x++, v++)
	//	{
	//		Triangles[i] = v;
	//		Triangles[i + 1] = Triangles[i + 4] = v + 1;
	//		Triangles[i + 2] = Triangles[i + 3] = v + Resolution - 2;
	//		Triangles[i + 5] = v + Resolution - 1;
	//		i += 6;
	//	}		
	//}
}


void AProceduralPlanetGenerator::SetNormals()
{
	FScopeLock Lock(&MeshGenCriticalSection);

	Normals.Empty();
}

