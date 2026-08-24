// Fill out your copyright notice in the Description page of Project Settings.
#include "Magic_A1.h"
#include <Kismet/GameplayStatics.h>
#include "PlayerCharacter.h"
#include "Tile.h"
#include <Kismet/KismetMathLibrary.h>

void AMagic_A1::BeginPlay()
{
	Super::BeginPlay();
    UPaperSprite* LoadedSprite = LoadObject<UPaperSprite>(nullptr, TEXT("/Game/GameObject/Sprite/C-1_Sprite.C-1_Sprite"));
    if (LoadedSprite)
    {
        Sprite->SetSprite(LoadedSprite);
    }

	//初期設定
	MagicName = TEXT("ヒノタマストレート");
	MagicEffectString = TEXT("前方にに火の玉を放つ。\nコスパ最強！\n消費MP：10");
	UseMP = 10;
	AttackPower = 10;
	MagicID = 1;

	//サウンド設定
	SoundEffect = LoadObject<USoundBase>(nullptr, TEXT("/Script/Engine.SoundWave'/Game/GameOhter/other/火炎魔法1.火炎魔法1'"));
	CastSound = LoadObject<USoundBase>(nullptr, TEXT("/Script/Engine.SoundWave'/Game/GameOhter/other/打撃2.打撃2'"));


	if (!Sprite)
	{
		UE_LOG(LogTemp, Error, TEXT("SpriteComponent is NULL"));
		return;
	}

	UPaperSprite* SpriteAsset = Sprite->GetSprite();
	if (!SpriteAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("Sprite->GetSprite() is NULL"));
		return;
	}

	IconTexture = SpriteAsset->GetBakedTexture();
	if (!IconTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("GetBakedTexture returned NULL"));
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("IconTexture is valid: %s"), *IconTexture->GetName());

}

void AMagic_A1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMagic_A1::MagicEffect()
{
    Super::MagicEffect();

	//マス目情報を取得
	TArray<AActor*> Tiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), Tiles);

	FRotator SpawnRotation = FRotator::ZeroRotator;

	//魔法が展開される円の半径
	float Radius = 300.0f;

	for (int i =0; i < Width; i++) {

		//マス目の情報を取得して、魔法の効果範囲と一致するマス目にむけて魔法弾を生成する
		for (AActor* Tile : Tiles){
			ATile* UseTile = Cast <ATile>(Tile);
			if (UseTile->x == i && UseTile->y==Player->Y && UseTile->side == false) {
			
				//魔法弾を円を書くように生成するための角度を計算
				float Angle = (2 * PI / Width) * i;

				FVector CharacterLocation = Player->GetActorLocation();
				CharacterLocation.X = CharacterLocation.X + Radius * FMath::Cos(Angle);
				CharacterLocation.Z = CharacterLocation.Z + Radius * FMath::Sin(Angle);
				
				AMagicBall* NewMagicBall = GetWorld()->SpawnActor<AMagicBall>(AMagicBall::StaticClass(), CharacterLocation, SpawnRotation);
				NewMagicBall->Target=UseTile->GetActorLocation();
				NewMagicBall->AnimationSprite->SetSpriteColor(FLinearColor(0.9f,0.1f,0.1f));

				//魔法弾の効果を設定
				NewMagicBall->TargetX = i;
				NewMagicBall->AttackPower = AttackPower;
				NewMagicBall->TargetY = Player->Y;
				NewMagicBall->TargetTile = false;

				//サウンドを設定
				NewMagicBall->AttackSoundEffect = CastSound;

				FVector MyLocation = GetActorLocation();
				FVector TargetLocation = UseTile->GetActorLocation();
				MyLocation.Y = 0;
				TargetLocation.Y = 0;
				FRotator NewRot = UKismetMathLibrary::FindLookAtRotation(MyLocation, TargetLocation);
				
				//終わったら消すこと
				//NewRot.Pitch += 90.0f;
				
				NewMagicBall->SetActorRotation(NewRot);
			}
		}
	}

	//MPを消費
	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	Player->mp = Player->mp - UseMP;

	//次に選択する付与魔法を展開
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMagic::StaticClass(), Magics);
	for (AActor* Magic : Magics)
	{
		AMagic* UseMagic = Cast <AMagic>(Magic);
		if (UseMagic->flagMagic == false) {
			UseMagic->StartCast();
		}
	}
}
