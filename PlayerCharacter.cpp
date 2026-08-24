// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "PassGameData.h"
#include "GameCameraActor.h"
#include "Magic.h"
#include "MagicBall.h"
#include "Magic_A1.h"
#include "Magic_A2.h"
#include "Magic_B1.h"
#include "Magic_B3.h"
#include "Magic_A3.h"
#include "Magic_A4.h"
#include "Magic_A6.h"
#include "Magic_B5.h"
#include "Magic_B4.h"
#include "Magic_A5.h"
#include "Magic_B2.h"
#include "Tile.h"
#include <Kismet/GameplayStatics.h>

APlayerCharacter::APlayerCharacter()
{

	PrimaryActorTick.bCanEverTick = true;

	// スプライトコンポーネントを作成してルートに設定
	Sprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	RootComponent = Sprite;

	//このアクターを動けるようにする。
	Sprite->SetMobility(EComponentMobility::Movable);

}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	//UE_LOG(LogTemp, Warning, TEXT("BeginPlay"));

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && GameCamera) {
		EnableInput(PC);
		PC->Possess(this);
		PC->SetViewTarget(GameCamera);
	}

	FVector SpawnLocation = GetActorLocation() + FVector(10000, 0, 10000); //見えない場所にスポーンさせる座標
	FRotator SpawnRotation = FRotator::ZeroRotator;

	//ゲームデータ取得
	UPassGameData* GameData = Cast<UPassGameData>(GetGameInstance());
	if (GameData)
	{
		Width = GameData->SelectedStageData.GridWidth;
		Height = GameData->SelectedStageData.GridHeight;

		//習得してる魔法が0なら初期魔法A-1とB-1を習得
		if (GameData->SelectedStageData.MagicNo_A.Num() == 0) {
			GameData->SelectedStageData.MagicNo_A.Add(1);
		}

		if (GameData->SelectedStageData.MagicNo_B.Num() == 0) {
			GameData->SelectedStageData.MagicNo_B.Add(1);
		}

		//習得魔法数のカウント
		int i=0;

		//基礎魔法
		for (int SpawnMagic : GameData->SelectedStageData.MagicNo_A)
		{
			// MagicNoからクラスを決定
			switch (SpawnMagic) {
			case 1:
				SpawnMagicClass = AMagic_A1::StaticClass();
				break;
			case 2:
				SpawnMagicClass = AMagic_A2::StaticClass();
				break;
			case 3:
				SpawnMagicClass = AMagic_A3::StaticClass();
				break;
			case 4:
				SpawnMagicClass = AMagic_A4::StaticClass();
				break;
			case 5:
				SpawnMagicClass = AMagic_A5::StaticClass();
				break;
			case 6:
				SpawnMagicClass = AMagic_A6::StaticClass();
				break;
			}
		
			//習得している魔法を待機
			AMagic* NewMagic = GetWorld()->SpawnActor<AMagic>(SpawnMagicClass, SpawnLocation, SpawnRotation);
			NewMagic->MagicNo = i;
			NewMagic->SetActorScale3D(SpellIconSize);
			NewMagic->Sprite->SetTranslucentSortPriority(4);

			i++;
		}
		i = 0;
		//付与魔法
		for (int SpawnMagic : GameData->SelectedStageData.MagicNo_B)
		{
			// MagicNoからクラスを決定
			switch (SpawnMagic) {
			case 1:
				SpawnMagicClass = AMagic_B1::StaticClass();
				break;
			case 2:
				SpawnMagicClass = AMagic_B2::StaticClass();
				break;
			case 3:
				SpawnMagicClass = AMagic_B3::StaticClass();
				break;
			case 4:
				SpawnMagicClass = AMagic_B4::StaticClass();
				break;
			case 5:
				SpawnMagicClass = AMagic_B5::StaticClass();
				break;
			}

			//習得している魔法を待機
			AMagic* NewMagic = GetWorld()->SpawnActor<AMagic>(SpawnMagicClass, SpawnLocation, SpawnRotation);
			NewMagic->SetActorScale3D(SpellIconSize);
			NewMagic->MagicNo = i;
			NewMagic->flagMagic = false;
			NewMagic->Sprite->SetTranslucentSortPriority(4);
			i++;
		}

	}
	
	//中央にセット
	X = Width / 2;
	Y = Height / 2;

	//HPの最大値取得
	if (GameData) {
		hp = GameData->SelectedStageData.PlayerHP;
	}

	//Tile（マス目）の位置を取得してプレイヤーの位置をセット
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), Tiles);
	for (AActor* Tile : Tiles)
	{
		ATile* UseTile = Cast <ATile>(Tile);
		if (UseTile->x == X && UseTile->y == Y && UseTile->side == true) {
			FVector TileLocation = UseTile->GetActorLocation();
			TileLocation.Y = TileLocation.Y + 10;
			SetActorLocation(TileLocation);

		}
	}

	//Z軸から見下ろすカメラならY軸回転で正面を向ける
	//Sprite->SetRelativeRotation(FRotator(90.0f, 90.0f, 90.0f));

	//描画順序を設定
	Sprite->SetTranslucentSortPriority(5);

	//各種バーの取得
	FVector BarLocation = GetActorLocation() + FVector(0.0f, 0.0f, 330.0f);
	HPBar = GetWorld()->SpawnActor<AHPBar>(AHPBar::StaticClass(), FVector(0.0f, 1.0f, 0.0f), FRotator::ZeroRotator);
	HPBar->MaxHp = hp;
	HPBar->barColor = FLinearColor(0.1f, 0.9f, 0.1f, 1.0f);
	HPBar->SetActorLocation(BarLocation);
	BarLocation.Z -= 40.0f;
	MPBar = GetWorld()->SpawnActor<AHPBar>(AHPBar::StaticClass(), FVector(0.0f, 1.0f, 0.0f), FRotator::ZeroRotator);
	MPBar->MaxHp = mp;
	MPBar->barColor = FLinearColor(0.1f, 0.1f, 0.9f, 1.0f);
	MPBar->SetActorLocation(BarLocation);

	//作り終わったら適宜消すこと
	//stop = false;

}
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("MoveUp", IE_Pressed, this, &APlayerCharacter::MoveUp);
	PlayerInputComponent->BindAction("MoveDown", IE_Pressed, this, &APlayerCharacter::MoveDown);
	PlayerInputComponent->BindAction("MoveRight", IE_Pressed, this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAction("MoveLeft", IE_Pressed, this, &APlayerCharacter::MoveLeft);
	PlayerInputComponent->BindAction("Space", IE_Pressed, this, &APlayerCharacter::Space);
	PlayerInputComponent->BindAction("Space", IE_Released, this, &APlayerCharacter::SpaceReleased);

}

void APlayerCharacter::MoveUp()
{
	if (stop == false && isCasting == false) {
		//座標を移動　越えてたりしたら近似値に
		Y--;
		Y = FMath::Clamp(Y, 0, Height - 1);

		//タイルを取得
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), Tiles);

		for (AActor* Tile : Tiles)
		{
			ATile* UseTile = Cast <ATile>(Tile);
			if (UseTile->x == X && UseTile->y == Y && UseTile->side == true) {
				FVector TileLocation = UseTile->GetActorLocation();
				TileLocation.Y = TileLocation.Y + 10;
				SetActorLocation(TileLocation);
			}
		}
	}
	else if (isCasting == true) {//詠唱中なら
		//展開中の魔法情報を取得
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMagic::StaticClass(), Magics);
		
		//基礎魔法詠唱済みなら　付与魔法を
		if (isCastingNormalSpell == true) {
			
			for (AActor* Magic : Magics) {
				
				AMagic* UseMagic = Cast <AMagic>(Magic);
				UseMagic->SetActorScale3D(SpellIconSize);
				
				if (UseMagic->MagicNo == 0 && UseMagic->flagMagic == false) {

					if (SelectedSpellNo == 0) {//選択中の魔法をもう一度選択したら発動
						UseMagic->MagicEffect();

						//選択中の魔法を解除
						SelectedSpellNo = -1;
						MagicName = TEXT("");
						MagicEffectString = TEXT("");
					}
					else {//魔法を選択＆拡大表示
						UseMagic->SetActorScale3D(SpellIconBigSize);
						SelectedSpellNo = UseMagic->MagicNo;
						//魔法の名前と効果を表示
						MagicName = UseMagic->MagicName;
						MagicEffectString = UseMagic->MagicEffectString;
					}
				}
			}
		}
		else {//基礎魔法詠唱前なら 基礎魔法を
			for (AActor* Magic : Magics) {

				AMagic* UseMagic = Cast <AMagic>(Magic);
				UseMagic->SetActorScale3D(SpellIconSize);
				
				if (UseMagic->MagicNo == 0 && UseMagic->flagMagic == true) {

					if (SelectedSpellNo == 0) {//選択中の魔法をもう一度選択したら発動
						UseMagic->MagicEffect();
						//基礎魔法詠唱済みフラグを立てる
						isCastingNormalSpell = true;

						//詠唱中フラグを下ろす
						SelectedSpellNo = -1;
						MagicName = TEXT("");
						MagicEffectString = TEXT("");
					}
					else {//魔法を選択＆拡大表示
						UseMagic->SetActorScale3D(SpellIconBigSize);
						SelectedSpellNo = UseMagic->MagicNo;

						//魔法の名前と効果を表示
						MagicName = UseMagic->MagicName;
						MagicEffectString = UseMagic->MagicEffectString;
					}
				}

			}
		}
	}
}

void APlayerCharacter::MoveDown()
{

	if (stop == false && isCasting == false) {
		//座標を移動　越えてたりしたら近似値に
		Y++;
		Y = FMath::Clamp(Y, 0, Height - 1);

		//タイルを取得
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), Tiles);

		for (AActor* Tile : Tiles)
		{
			ATile* UseTile = Cast <ATile>(Tile);
			if (UseTile->x == X && UseTile->y == Y && UseTile->side == true) {
				FVector TileLocation = UseTile->GetActorLocation();
				TileLocation.Y = TileLocation.Y + 10;
				SetActorLocation(TileLocation);
			
			}

		}
	}
	else if (isCasting == true) {
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMagic::StaticClass(), Magics);
		if (isCastingNormalSpell == true) {
			for (AActor* Magic : Magics) {
				AMagic* UseMagic = Cast <AMagic>(Magic);
				UseMagic->SetActorScale3D(SpellIconSize);
				if (UseMagic->MagicNo == 2 && UseMagic->flagMagic == false) {
					if (SelectedSpellNo == 2) {
						UseMagic->MagicEffect();
						SelectedSpellNo = -1;
						MagicName = TEXT("");
						MagicEffectString = TEXT("");
					}
					else {
						UseMagic->SetActorScale3D(SpellIconBigSize);
						SelectedSpellNo = UseMagic->MagicNo;
						MagicName = UseMagic->MagicName;
						MagicEffectString = UseMagic->MagicEffectString;
					}
				}

			}
		}
		else {
			for (AActor* Magic : Magics) {
				AMagic* UseMagic = Cast <AMagic>(Magic);
				UseMagic->SetActorScale3D(SpellIconSize);
				if (UseMagic->MagicNo == 2 && UseMagic->flagMagic == true) {
					if (SelectedSpellNo == 2) {
						UseMagic->MagicEffect();
						isCastingNormalSpell = true;
						SelectedSpellNo = -1;
						MagicName = TEXT("");
						MagicEffectString = TEXT("");
					}
					else {
						UseMagic->SetActorScale3D(SpellIconBigSize);
						SelectedSpellNo = UseMagic->MagicNo;
						MagicName = UseMagic->MagicName;
						MagicEffectString = UseMagic->MagicEffectString;
					}
				}

			}
		}
	}
}

void APlayerCharacter::MoveLeft()
{

	if (stop == false && isCasting==false) {
		//座標を移動　越えてたりしたら近似値に
		X++;
		X = FMath::Clamp(X, 0, Width - 1);

		//タイルを取得
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), Tiles);

		for (AActor* Tile : Tiles)
		{
			ATile* UseTile = Cast <ATile>(Tile);
			if (UseTile->x == X && UseTile->y == Y && UseTile->side == true) {
				FVector TileLocation = UseTile->GetActorLocation();
				TileLocation.Y = TileLocation.Y + 10;
				SetActorLocation(TileLocation);
			
			}

		}
	}
	else if (isCasting == true) {
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMagic::StaticClass(), Magics);
		if (isCastingNormalSpell == true) {
			for (AActor* Magic : Magics) {
				AMagic* UseMagic = Cast <AMagic>(Magic);
				UseMagic->SetActorScale3D(SpellIconSize);
				if (UseMagic->MagicNo == 1 && UseMagic->flagMagic == false) {
					if (SelectedSpellNo == 1) {
						UseMagic->MagicEffect();
						SelectedSpellNo = -1;
						MagicName = TEXT("");
						MagicEffectString = TEXT("");
					}
					else {
						UseMagic->SetActorScale3D(SpellIconBigSize);
						SelectedSpellNo = UseMagic->MagicNo;
						MagicName = UseMagic->MagicName;
						MagicEffectString = UseMagic->MagicEffectString;
					}
				}

			}
		}
		else {
			for (AActor* Magic : Magics) {
				AMagic* UseMagic = Cast <AMagic>(Magic);
				UseMagic->SetActorScale3D(SpellIconSize);
				if (UseMagic->MagicNo == 1 && UseMagic->flagMagic == true) {
					if (SelectedSpellNo == 1) {
						UseMagic->MagicEffect();
						isCastingNormalSpell = true;
						SelectedSpellNo = -1;
						MagicName = TEXT("");
						MagicEffectString = TEXT("");
					}
					else {
						UseMagic->SetActorScale3D(SpellIconBigSize);
						SelectedSpellNo = UseMagic->MagicNo;
						MagicName = UseMagic->MagicName;
						MagicEffectString = UseMagic->MagicEffectString;
					}
				}

			}
		}
	}
}

void APlayerCharacter::MoveRight()
{

	if (stop == false && isCasting == false) {
		//座標を移動　越えてたりしたら近似値に
		X--;
		X = FMath::Clamp(X, 0, Width - 1);

		//タイルを取得
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), Tiles);

		for (AActor* Tile : Tiles)
		{
			ATile* UseTile = Cast <ATile>(Tile);
			if (UseTile->x == X && UseTile->y == Y && UseTile->side == true) {
				FVector TileLocation = UseTile->GetActorLocation();
				TileLocation.Y = TileLocation.Y + 10;
				SetActorLocation(TileLocation);
			}

		}
	}
	else if (isCasting == true) {
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMagic::StaticClass(), Magics);
		if (isCastingNormalSpell == true) {
			for (AActor* Magic : Magics) {
				AMagic* UseMagic = Cast <AMagic>(Magic);
				UseMagic->SetActorScale3D(SpellIconSize);
				if (UseMagic->MagicNo == 3 && UseMagic->flagMagic == false) {
					if (SelectedSpellNo == 3) {
						UseMagic->MagicEffect();
						SelectedSpellNo = -1;
						MagicName = TEXT("");
						MagicEffectString = TEXT("");
					}
					else {
						UseMagic->SetActorScale3D(SpellIconBigSize);
						SelectedSpellNo = UseMagic->MagicNo;
						MagicName = UseMagic->MagicName;
						MagicEffectString = UseMagic->MagicEffectString;
					}
				}

			}
		}
		else {
			for (AActor* Magic : Magics) {
				AMagic* UseMagic = Cast <AMagic>(Magic);
				UseMagic->SetActorScale3D(SpellIconSize);
				if (UseMagic->MagicNo == 3 && UseMagic->flagMagic == true) {
					if (SelectedSpellNo == 3) {
						UseMagic->MagicEffect();
						isCastingNormalSpell = true;
						SelectedSpellNo = -1;
						MagicName = TEXT("");
						MagicEffectString = TEXT("");
					}
					else {
						UseMagic->SetActorScale3D(SpellIconBigSize);
						SelectedSpellNo = UseMagic->MagicNo;
						MagicName = UseMagic->MagicName;
						MagicEffectString = UseMagic->MagicEffectString;
					}
				}

			}
		}
	}
}

void APlayerCharacter::Space()
{
	if (stop == false) {
		//詠唱中フラグを立てる
		isCasting = true;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMagic::StaticClass(), Magics);
		
		//習得してる魔法を表示する
		for (AActor* Magic : Magics)
		{
			AMagic* UseMagic = Cast <AMagic>(Magic);
			if (UseMagic->flagMagic == true&&UseMagic->flagLearn==false) {
				UseMagic->StartCast();
			}
		}
	}
}

void APlayerCharacter::SpaceReleased()
{
	//詠唱中フラグを下ろす
	isCastingNormalSpell = false;
	isCasting = false;

	//選択中の魔法を解除
	SelectedSpellNo = -1;
	MagicName = TEXT("");
	MagicEffectString = TEXT("");
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMagicBall::StaticClass(), MagicBalls);

	//付与魔法待機中の魔法弾を放つ
	for (AActor* MagicBall : MagicBalls)
	{
		AMagicBall* UseMagicBall = Cast <AMagicBall>(MagicBall);
		if (UseMagicBall->Stey == true && UseMagicBall->TargetTile == false) {
			UseMagicBall->Stey = false;
		}
	}

	//習得してる魔法を非表示にする
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMagic::StaticClass(), Magics);
	for (AActor* Magic : Magics)
	{
		AMagic* UseMagic = Cast <AMagic>(Magic);
		if (UseMagic->flagLearn == false) {
			UseMagic->SetActorLocation(FVector(10000, 0, 10000));
			UseMagic->SetActorScale3D(SpellIconSize);
		}
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	//毎秒MP回復
	mp = mp + PrassMagicPower;
	
	Super::Tick(DeltaTime);
	Sprite->SetRelativeRotation(FRotator(0.0f, 0.0f,0.0f));
	if (mp < 0) {//MPが0以下になったら詠唱中止
		mp = 0;
		isCasting = false;
		isCastingNormalSpell = false;

		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMagicBall::StaticClass(), MagicBalls);

		//付与魔法待機中の魔法弾を消す
		for (AActor* MagicBall : MagicBalls)
		{
			AMagicBall* UseMagicBall = Cast <AMagicBall>(MagicBall);
			if (UseMagicBall->Stey == true && UseMagicBall->TargetTile == false) {
				UseMagicBall->Destroy();
			}
		}
		//習得してる魔法を非表示にする
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMagic::StaticClass(), Magics);
		for (AActor* Magic : Magics)
		{
			AMagic* UseMagic = Cast <AMagic>(Magic);
			if (UseMagic->flagLearn == false) {
				UseMagic->SetActorLocation(FVector(10000, 0, 10000));
			}
		}
	}
	//HP・MPの上限下限の制御
	if (hp < 0) {
		hp = 0;
	}
	else if(hp>MaxHP){
		hp = MaxHP;
	}
	if(mp > MaxMP) {
		mp = MaxMP;
	}
	else if(mp < 0) {
		mp = 0;
	}

	if(hp<=0){
		//ゲームオーバー処理
		stop = true;
	}
	HPBar->hp = hp;
	MPBar->hp = mp;

	FVector BarLocation = GetActorLocation() + FVector(0.0f, 0.0f, 330.0f);
	HPBar->SetActorLocation(BarLocation);
	BarLocation.Z -= 40.0f;
	MPBar->SetActorLocation(BarLocation);
}

//クリックされたら
void APlayerCharacter::OnClick()
{
	//終わったら消すこと
	//UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter OnClick called!"));
}

void APlayerCharacter::NewCards()
{
	//新しい魔法カードを生成する処理
	FVector SpawnLocation = GetActorLocation() + FVector(10000, 0, 10000);
	FRotator SpawnRotation = FRotator::ZeroRotator;

	// ゲームデータ取得
	UPassGameData* GameData = Cast<UPassGameData>(GetGameInstance());
	if (!GameData)
	{
		UE_LOG(LogTemp, Error, TEXT("GameData が取得できませんでした"));
		return;
	}

	struct FCandidateMagic
	{
		int32 MagicType; // 0 = 基礎魔法, 1 = 付与魔法
		int32 MagicID;//識別番号
	};

	// 抽選候補リスト
	TArray<FCandidateMagic> Candidates;

	//基礎魔法候補追加
	for (int32 i = 1; i <= GameData->SelectedStageData.MagicCount_A; i++)
	{
		// まだ習得していないものだけ追加
		if (!GameData->SelectedStageData.MagicNo_A.Contains(i))
		{
			Candidates.Add({ 0, i });
		}
	}

	//付与魔法候補追加
	for (int32 i = 1; i <= GameData->SelectedStageData.MagicCount_B; i++)
	{
		if (!GameData->SelectedStageData.MagicNo_B.Contains(i))
		{
			Candidates.Add({ 1, i });
		}
	}

	// 抽選候補がない場合の処理
	if (Candidates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("抽選候補がありません"));
		return;
	}

	//抽選
	int32 MaxChoices = FMath::Min(GameData->SelectedStageData.MagicChoiceCount, Candidates.Num());

	for (int32 i = 0; i < MaxChoices; i++){

		if (Candidates.Num() == 0)
			break;

		int32 Index = FMath::RandRange(0, Candidates.Num() - 1);
		FCandidateMagic Pick = Candidates[Index];

		bool MagicFlug = true;

		switch (Pick.MagicType)
		{
		case 0: // 基礎魔法
			switch (Pick.MagicID)
			{
			case 1: SpawnMagicClass = AMagic_A1::StaticClass(); break;
			case 2: SpawnMagicClass = AMagic_A2::StaticClass(); break;
			case 3: SpawnMagicClass = AMagic_A3::StaticClass(); break;
			case 4: SpawnMagicClass = AMagic_A4::StaticClass(); break;
			case 5: SpawnMagicClass = AMagic_A5::StaticClass(); break;
			case 6: SpawnMagicClass = AMagic_A6::StaticClass(); break;
			}
			break;

		case 1: // 付与魔法
			switch (Pick.MagicID)
			{
			case 1: SpawnMagicClass = AMagic_B1::StaticClass(); MagicFlug = false; break;
			case 2: SpawnMagicClass = AMagic_B2::StaticClass(); MagicFlug = false; break;
			case 3: SpawnMagicClass = AMagic_B3::StaticClass(); MagicFlug = false; break;
			case 4: SpawnMagicClass = AMagic_B4::StaticClass(); MagicFlug = false; break;
			case 5: SpawnMagicClass = AMagic_B4::StaticClass(); MagicFlug = false; break;
			}
			break;
		}
		//スポーン
		AMagic* NewMagic = GetWorld()->SpawnActor<AMagic>(SpawnMagicClass, SpawnLocation, SpawnRotation);
		if (NewMagic){

			NewMagic->SetActorScale3D(SpellIconSize);
			NewMagic->Sprite->SetTranslucentSortPriority(4);
			NewMagic->flagLearn = true;
			NewMagic->flagMagic = MagicFlug;
			NewMagic->MagicNo = 99;
		}
		// 選ばれた要素を削除（重複防止）
		Candidates.RemoveAt(Index);
	}
}
/*
void APlayerCharacter::NewCards()
{
	FVector SpawnLocation = GetActorLocation() + FVector(10000, 0, 10000);
	FRotator SpawnRotation = FRotator::ZeroRotator;

	//ゲームデータ取得
	UPassGameData* GameData = Cast<UPassGameData>(GetGameInstance());
	//習得する基礎魔法の抽選
	TArray<int32> Number;
	for (int32 i = 1; i <= GameData->SelectedStageData.MagicCount_A; i++)
	{
		if (!GameData->SelectedStageData.MagicNo_A.Contains(i))
		{
			Number.Add(i);
		}
	}

	if (Number.Num() == 0) {}
	else {
		for (int32 i = 0; i < GameData->SelectedStageData.MagicChoiceCount; i++)
		{
			if (Number.Num() == 0)
			{
				break; // もう取れるものがない
			}

			int32 Index = FMath::RandRange(0, Number.Num() - 1);

			// 処理
			switch (Number[Index]) {
			case 1:
				SpawnMagicClass = AMagic_A1::StaticClass();
				break;
			case 2:
				SpawnMagicClass = AMagic_A2::StaticClass();
				break;
			}
			//スポーン
			AMagic* NewMagic = GetWorld()->SpawnActor<AMagic>(SpawnMagicClass, SpawnLocation, SpawnRotation);
			NewMagic->SetActorScale3D(SpellIconSize);
			NewMagic->Sprite->SetTranslucentSortPriority(4);
			NewMagic->flagLearn = true;

			NewMagic->MagicNo = 99;


			// 選ばれた要素を削除（重複防止）
			Number.RemoveAt(Index);
		}

	}
	//習得する付与魔法の抽選
	TArray<int32> Number_2;
	for (int32 i = 1; i <= GameData->SelectedStageData.MagicCount_B; i++)
	{
		if (!GameData->SelectedStageData.MagicNo_B.Contains(i))
		{
			Number_2.Add(i);
		}
	}

	if (Number_2.Num() == 0) {}
	else {
		for (int32 i = 0; i < GameData->SelectedStageData.MagicChoiceCount; i++)
		{
			if (Number_2.Num() == 0)
			{
				break; // もう取れるものがない
			}

			int32 Index = FMath::RandRange(0, Number_2.Num() - 1);

			// 処理
			switch (Number_2[Index]) {
			case 1:
				SpawnMagicClass = AMagic_B1::StaticClass();
				break;
			}
			//スポーン
			AMagic* NewMagic = GetWorld()->SpawnActor<AMagic>(SpawnMagicClass, SpawnLocation, SpawnRotation);
			NewMagic->SetActorScale3D(SpellIconSize);
			NewMagic->Sprite->SetTranslucentSortPriority(4);
			NewMagic->flagMagic = false;
			NewMagic->flagLearn = true;

			NewMagic->MagicNo = 99;


			// 選ばれた要素を削除（重複防止）
			Number_2.RemoveAt(Index);
		}

	}
}
*/