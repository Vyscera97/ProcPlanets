#include "CubeSphere.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Math/UnrealMathUtility.h"


// Sets default values
ACubeSphere::ACubeSphere()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CubeSphere"));
	RootComponent = Mesh;

	UE_ENABLE_DEBUG_DRAWING;
}

void ACubeSphere::GenerateMesh()
{
	//FScopeLock Lock(&MeshGenCriticalSection);

	if (Radius == 0 || VertexDistance == 0) { return; }

	Resolution = 2 * Radius / VertexDistance;

	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	UV0.Empty();
	VertexColors.Empty();
	Tangents.Empty();

	CreateVertices();	

	for (int v = 0; v < Vertices.Num(); v++)
	{
		Vertices[v] -= FVector(Radius-(VertexDistance * 0.5), Radius - (VertexDistance * 0.5), Radius - (VertexDistance * 0.5));
		Vertices[v] /= Radius - (VertexDistance * 0.5);
		Vertices[v].Normalize();
		Vertices[v] *= Radius - (VertexDistance * 0.5);
		DrawDebugSphere(GetWorld(), Vertices[v] + GetActorLocation(), 10.0f, 6, FColor::Magenta, true);
	}
	
	Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);
	/*AsyncTask(ENamedThreads::GameThread, [this]()
		{
			
		});*/
}

void ACubeSphere::CreateVertices()
{
	//FScopeLock Lock(&MeshGenCriticalSection);

	Vertices.SetNum(6 * Resolution * Resolution);
	int v = 0;
	int Offset = 3 * Resolution * Resolution;
	int Max = (Resolution - 1) * VertexDistance;

	for (int z = 0; z < Resolution; z++)
	{
		for (int x = 0; x < Resolution; x++, v++)
		{
			int32 xPos = x * VertexDistance;
			int32 zPos = z * VertexDistance;
			Vertices[v] = FVector(xPos, 0, zPos);
			Vertices[v + Offset] = FVector(xPos, Max, zPos);
		}
	}
	for (int z = 0; z < Resolution; z++)
	{
		for (int y = 0; y < Resolution; y++, v++)
		{
			int32 yPos = y * VertexDistance;
			int32 zPos = z * VertexDistance;
			Vertices[v] = FVector(0, yPos, zPos);
			Vertices[v + Offset] = FVector(Max, yPos, zPos);
		}
	}	
	for (int y = 0; y < Resolution; y++)
	{
		for (int x = 0; x < Resolution; x++, v++)
		{
			int32 xPos = x * VertexDistance;
			int32 yPos = y * VertexDistance;
			Vertices[v] = FVector(xPos, yPos, 0);
			Vertices[v + Offset] = FVector(xPos, yPos, Max);
		}
	}

	CreateTriangles();

	
}
void ACubeSphere::CreateTriangles()
{
	Triangles.SetNum(6 * 6 * (Resolution - 1) * (Resolution - 1));

	int t = 0;
	int Vertex = 0;
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < Resolution - 1; j++, Vertex++)
		{
			for (int k = 0; k < Resolution - 1; k++, Vertex++)
			{
				Triangles[t] = Vertex;
				Triangles[t + 1] = Triangles[t + 4] = Vertex + Resolution;
				Triangles[t + 2] = Triangles[t + 3] = Vertex + 1;
				Triangles[t + 5] = Vertex + Resolution + 1;
				if (i > 0 && i < 4)
				{
					Triangles[t + 1] = Triangles[t + 4] = Vertex + 1;
					Triangles[t + 2] = Triangles[t + 3] = Vertex + Resolution;
				}
				t += 6;
			}
		};
		Vertex = Resolution * Resolution * (i+1);
	}
	
}
//
//void ACubeSphere::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
//{
//	Super::PostEditChangeProperty(PropertyChangedEvent);
//
//	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
//		{
//			GenerateMesh();
//		});
//}
//
//void ACubeSphere::OnConstruction(const FTransform& Transform)
//{
//	Super::OnConstruction(Transform);
//
//
//	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
//		{
//			GenerateMesh();
//		});
//}

// Called when the game starts or when spawned
void ACubeSphere::BeginPlay()
{
	Super::BeginPlay();

	GenerateMesh();
	
}

// Called every frame
void ACubeSphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}




