#include "TimeOfDayEditor.h"
#include "Toolkits/IToolkitHost.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"

#include "Editor/PropertyEditor/Private/PropertyChangeListener.h"
#include "Editor/PropertyEditor/Public/IPropertyChangeListener.h"

#include "Editor/PropertyEditor/Public/IDetailsView.h"

#include "Editor/UnrealEd/Public/SCurveEditor.h"

#include "TODAssetDetails.h"
#include "TODAssetPropertyDetails.h"
#include "Runtime/Slate/Public/Widgets/Docking/SDockTab.h"
#include "TODAssetEditor.h"

const FName FTODAssetEditor::TODEditorTabId(TEXT("TimeOfDayEditor_TabId")); //curve properties names
const FName FTODAssetEditor::TODEditorTabDetails(TEXT("TimeOfDayEditor_Details")); //curves
const FName FTODAssetEditor::TODEditorTabBaseProperties(TEXT("TimeOfDayEditor_BaseProperties")); //base (non curve) properties
void FTODAssetEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(TabManager);
	const IWorkspaceMenuStructure& MenuStructure = WorkspaceMenu::GetMenuStructure();

	TabManager->RegisterTabSpawner(TODEditorTabId, FOnSpawnTab::CreateSP(this, &FTODAssetEditor::SpawnTab_Item))
		.SetDisplayName(FText::FromString("Time Of Day Editor"))
		.SetGroup(MenuStructure.GetLevelEditorDetailsCategory());

	TabManager->RegisterTabSpawner(TODEditorTabDetails, FOnSpawnTab::CreateSP(this, &FTODAssetEditor::SpawnTab_Details))
		.SetDisplayName(FText::FromString("Item Details"))
		.SetGroup(MenuStructure.GetLevelEditorDetailsCategory());

	TabManager->RegisterTabSpawner(TODEditorTabBaseProperties, FOnSpawnTab::CreateSP(this, &FTODAssetEditor::SpawnTab_BaseProperties))
		.SetDisplayName(FText::FromString("Base Properties"))
		.SetGroup(MenuStructure.GetLevelEditorDetailsCategory());
}
void FTODAssetEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(TabManager);
	TabManager->UnregisterTabSpawner(TODEditorTabId);
	TabManager->UnregisterTabSpawner(TODEditorTabDetails);
	TabManager->UnregisterTabSpawner(TODEditorTabBaseProperties);
}
FTODAssetEditor::FTODAssetEditor()
{
	SelectedFloatCurve = nullptr;
	SelectedColorCurve = nullptr;
}
FTODAssetEditor::~FTODAssetEditor()
{
	SelectedFloatCurve = nullptr;
	SelectedColorCurve = nullptr;
}
void FTODAssetEditor::InitiItemEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UTODAsset* ItemIn)
{
	EditedItemAsset = ItemIn;
	SelectedFloatCurve = nullptr;
	SelectedColorCurve = nullptr;
	//SelectedColorCurve = &EditedItemAsset->SunColorCurve;
	SelectedFloatCurve = &EditedItemAsset->SunIntensityCurve;
	const bool bIsUpdatable = true;
	const bool bAllowFavorites = true;
	const bool bIsLockable = false;
	const bool bAllowSearch = true;
	const bool bObjectsUseNameArea = false;
	const bool bHideSelectionTip = false;
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailViewArgs(bIsUpdatable, bIsLockable, bAllowSearch, bObjectsUseNameArea, bHideSelectionTip);

	ItemDetailsView = PropertyEditorModule.CreateDetailView(DetailViewArgs);

	//FOnGetCurrentItemIndex OnGetCurrentItemIndex = FOnGetCurrentItemIndex::CreateSP(this, &FARItemEditor::GetCurrentIndex);
	//FOnGetDetailCustomizationInstance LayoutVariableDetails = FOnGetDetailCustomizationInstance::CreateStatic(&FARItemAssetDetails::MakeInstance, OnGetCurrentItemIndex);
	
	FOnGetCurrentProperty OnGetCurrentProperty = FOnGetCurrentProperty::CreateSP(this, &FTODAssetEditor::GetCurrentProperty);
	
	FOnGetDetailCustomizationInstance LayoutVariableDetails = FOnGetDetailCustomizationInstance::CreateStatic(&FTODAssetDetails::MakeInstance, OnGetCurrentProperty);
	ItemDetailsView->RegisterInstancedCustomPropertyLayout(UTODAsset::StaticClass(), LayoutVariableDetails);
	ItemDetailsView->SetObject(ItemIn);

	PropertyDetailsView = PropertyEditorModule.CreateDetailView(DetailViewArgs);
	FOnGetDetailCustomizationInstance PropertyDetailsLayoutView = FOnGetDetailCustomizationInstance::CreateStatic(&FTODAssetPropertyDetails::MakeInstance);
	PropertyDetailsView->RegisterInstancedCustomPropertyLayout(UTODAsset::StaticClass(), PropertyDetailsLayoutView);
	PropertyDetailsView->SetObject(ItemIn);
	
	

	//ItemDetailsView->OnFinishedChangingProperties().AddRaw(this, &FARItemEditor::SaveEditedPRoperties);

	const TSharedRef<FTabManager::FLayout> TimeOfDayEditorDefault = FTabManager::NewLayout("Standalone_ItemEditor")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.2f)
				->SetHideTabWell(true)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.8f)
				->Split
				(
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.2f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.25f)
						->AddTab(TODEditorTabBaseProperties, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.5f)
						->AddTab(TODEditorTabDetails, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.25f)
						->AddTab(TODEditorTabId, ETabState::OpenedTab)
					)
				)
			)
		);


	//ItemEntry = SNew(SItemListWidget)
	//	.OnItemSelected(this, &FARItemEditor::HandleBlackboardEntrySelected)
	//	.ItemAsset(EditedItemAsset);
	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	InitAssetEditor(Mode, InitToolkitHost, FName("Time Of Day Editor")
		, TimeOfDayEditorDefault, true, true,
		ItemIn);

	RegenerateMenusAndToolbars();
}
FName FTODAssetEditor::GetCurrentProperty()
{
	return NAME_None;
}
// FAssetEditorToolkit
FName FTODAssetEditor::GetToolkitFName() const
{
	return FName("TimeOfDayEditor");
}
FText FTODAssetEditor::GetBaseToolkitName() const
{
	return FText::FromString("Time Of Day Editor");
}
FText FTODAssetEditor::GetToolkitName() const
{
	return FText::FromString("Time Of Day Editor");
}
FLinearColor FTODAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Red;
}
FString FTODAssetEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("TODEditor");
}

FString FTODAssetEditor::GetDocumentationLink() const
{
	return FString("No Link!");
}
// End of FAssetEditorToolkit

/** FCurveOwnerInterface Begin */
TArray<FRichCurveEditInfoConst> FTODAssetEditor::GetCurves() const
{
	TArray<FRichCurveEditInfoConst> Curves;
	if (SelectedFloatCurve)
	{
		Curves.Add(FRichCurveEditInfoConst(&SelectedFloatCurve->EditorCurveData));
	}
	if (SelectedColorCurve)
	{
		for (int32 Index = 0; Index < 4; Index++)
		{
			Curves.Add(FRichCurveEditInfoConst(&SelectedColorCurve->ColorCurves[Index]));
		}
	}
	return Curves;
}

TArray<FRichCurveEditInfo> FTODAssetEditor::GetCurves()
{
	TArray<FRichCurveEditInfo> CurveInfos;
	if (SelectedFloatCurve)
	{
		CurveInfos.Add(FRichCurveEditInfo(&SelectedFloatCurve->EditorCurveData));
	}
	if (SelectedColorCurve)
	{
		for (int32 Index = 0; Index < 4; Index++)
		{
			CurveInfos.Add(FRichCurveEditInfo(&SelectedColorCurve->ColorCurves[Index]));
		}
	}
	return CurveInfos;
}

UObject* FTODAssetEditor::GetOwner()
{
	return EditedItemAsset;// ? Will check
}

void FTODAssetEditor::ModifyOwner()
{
	if (EditedItemAsset)
	{
		EditedItemAsset->Modify(true);
	}
}

void FTODAssetEditor::MakeTransactional()
{
	if (EditedItemAsset)
	{
		EditedItemAsset->SetFlags(EditedItemAsset->GetFlags() | RF_Transactional);
	}
}

void FTODAssetEditor::OnCurveChanged()
{
	//commit changes to original curve
}

bool FTODAssetEditor::IsLinearColorCurve() const
{
	if (SelectedFloatCurve)
	{
		return false;
	}
	if (SelectedColorCurve)
	{
		return true;
	}
	return false;
}

FLinearColor FTODAssetEditor::GetLinearColorValue(float InTime) const
{
	if (SelectedColorCurve)
	{
		return SelectedColorCurve->GetLinearColorValue(InTime);
	}
	return FLinearColor::Black;
}

bool FTODAssetEditor::HasAnyAlphaKeys() const
{
	if (SelectedColorCurve)
	{
		return SelectedColorCurve->ColorCurves[3].GetNumKeys() > 0;
	}
	return false;
}
/* FCurveOwnerInterface End **/

void FTODAssetEditor::SetCurveToEdit()
{
	//reset pointers
	SelectedFloatCurve = nullptr;
	SelectedColorCurve = nullptr;

	if (CurveEditorWidget.IsValid())
	{
		CurveEditorWidget->SetCurveOwner(this, true);
	}
}

TSharedRef<SDockTab> FTODAssetEditor::SpawnTab_Item(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId().TabType == TODEditorTabId);

	return SNew(SDockTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				PropertyDetailsView.ToSharedRef()
			]
		];
}

TSharedRef<SDockTab> FTODAssetEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId().TabType == TODEditorTabDetails);


	TSharedRef<SDockTab> NewDockTab = SNew(SDockTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SAssignNew(CurveEditorWidget, SCurveEditor)
				.AlwaysDisplayColorCurves(true)
				//PropertyDetailsView.ToSharedRef()
			]
		];

	FCurveOwnerInterface* CurveOwner = this;
	if (CurveEditorWidget.IsValid())
	{
		CurveEditorWidget->SetCurveOwner(CurveOwner, true);
	}
	
	return NewDockTab;
}
TSharedRef<SDockTab> FTODAssetEditor::SpawnTab_BaseProperties(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId().TabType == TODEditorTabBaseProperties);

	return SNew(SDockTab)
		[
			SNew(SVerticalBox)
		];
}