#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralPlanetGenerator.generated.h"

UCLASS()
class PROCPLANETS_API AProceduralPlanetGenerator : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Mesh)
	bool bShouldUpdate;

	UPROPERTY(EditAnywhere, Category = Mesh)
	bool bIsCube;

	UPROPERTY(EditAnywhere, Category = Mesh)
	bool bIsRounded;

	UPROPERTY(EditAnywhere, Category = Mesh)
	int32 X_Size;

	UPROPERTY(EditAnywhere, Category = Mesh)
	int32 Y_Size;

	UPROPERTY(EditAnywhere, Category = Mesh)
	int32 Z_Size;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	int32 X_Bounds;
	UPROPERTY(VisibleAnywhere, Category = Mesh)
	int32 Y_Bounds;
	UPROPERTY(VisibleAnywhere, Category = Mesh)
	int32 Z_Bounds;

	UPROPERTY(EditAnywhere, Category = Mesh)	
	int32 StepSize;

	UPROPERTY(EditAnywhere, Category = Mesh, meta = (ClampMin = "1", ClampMax = "2"))
	float RoundnessScale;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	float Roundness;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TArray<FVector> Vertices;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TArray<int32> Triangles;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TArray<FVector> Normals;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TArray<FVector2D> UV0;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TArray<FLinearColor> VertexColors;

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	TArray<FProcMeshTangent> Tangents;
	
public:	
	// Sets default values for this actor's properties
	AProceduralPlanetGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	class UProceduralMeshComponent* ProceduralMesh;

	void GenerateMesh();
	void CreateVertices();
	void SetVertices(int i, int x, int y, int z);
	void CreateTriangles();
	void SetNormals();


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform& Transform) override;

private:
	FCriticalSection MeshGenCriticalSection;
};
