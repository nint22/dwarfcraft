/***************************************************************
 
 DwarfCraft - Dwarf Fortress / Minecraft clone
 Copyright 2011 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
***************************************************************/

// Includes
#include "g2EntityController.h"

static const int g2EntityController_Width = 300;
static const int g2EntityController_Height = 350;

g2EntityController::g2EntityController(g2Controller* Owner, g2Theme* MainTheme)
    : g2Panel(Owner, MainTheme)
{
    // Default to no known entity
    SelectedEntity = NULL;
    
    // Allocate all labels
    NameText = this->AddLabel(0, 0, "");
    NameText->SetColor(0, 0, 0);
    GenderAgeText = this->AddLabel(0, 0, "");
    GenderAgeText->SetColor(0, 0, 0);
    JobText = this->AddLabel(0, 0, "");
    JobText->SetColor(0, 0, 0);
    RankText = this->AddLabel(0, 0, "");
    RankText->SetColor(0, 0, 0);
    HealthText = this->AddLabel(0, 0, "");
    HealthText->SetColor(0, 0, 0);
    HappinessText = this->AddLabel(0, 0, "");
    HappinessText->SetColor(0, 0, 0);
    BreathHungerThirstText = this->AddLabel(0, 0, "");
    BreathHungerThirstText->SetColor(0, 0, 0);
    CurrentTaskText = this->AddLabel(0, 0, "");
    CurrentTaskText->SetColor(0, 0, 0);
    WorkPreferenceText = this->AddLabel(0, 0, "");
    WorkPreferenceText->SetColor(0, 0, 0);
    
    // Work types (primary and secondary)
    const char* WorkOptions[4] = {
        "No Preference",
        "Farmer",
        "Miner",
        "Crafter",
    };
    const char* SecondOptions[4] = {
        "No Preference",
        "Combat",
        "Medic",
        "Clerk",
    };
    
    // Interactive elements
    WorkPreferenceGroup = this->AddRadioGroup(0, 0, WorkOptions, 4);
    WorkPreferenceGroup->SetCallback(WorkPrefSet);
    
    SecondPreferenceGroup = this->AddRadioGroup(0, 0, SecondOptions, 4);
    SecondPreferenceGroup->SetCallback(SubWorkPrefSet);
    
    ShowPath = this->AddCheckBox(0, 0, "Render entitie's path");
    ShowPath->SetCallback(PathToggle);
    
    // Make sure this panel is floating
    this->SetTitle("Entity Observer");
    this->SetSize(g2EntityController_Width, g2EntityController_Height);
}

g2EntityController::~g2EntityController()
{
    // All auto-releases
}

void g2EntityController::SetDwarf(DwarfEntity* Entity)
{
    // Keep this copy, but ignore if null
    SelectedEntity = Entity;
    if(SelectedEntity == NULL)
        return;
    
    // Force an info commit first
    Update(0.0f);
    
    // Set some form title's
    WorkPreferenceText->SetText("Primary Job                  Secondary Job");
    ShowPath->GetLabel()->SetText("Render entitie's path");
    ShowPath->SetChecked(SelectedEntity->GetPathRender());
    
    // Sprite size and form origin
    Vector2<int> SpriteSize = SelectedEntity->GetPixelSize() + Vector2<int>(0, 10); // Shift down a little
    int VerticalSpacing = GetTheme()->GetCharacterHeight() + 6;
    
    // Place certain elements at predefined positions
    NameText->SetPos(g2EntityController_Width / 2 - NameText->GetWidth() / 2, SpriteSize.y + 1 * VerticalSpacing);
    GenderAgeText->SetPos(g2EntityController_Width / 2 - GenderAgeText->GetWidth() / 2, SpriteSize.y + 2 * VerticalSpacing);
    JobText->SetPos(10, SpriteSize.y + 4 * VerticalSpacing);
    RankText->SetPos(10, SpriteSize.y + 5 * VerticalSpacing);
    HealthText->SetPos(10, SpriteSize.y + 6 * VerticalSpacing);
    HappinessText->SetPos(10, SpriteSize.y + 7 * VerticalSpacing);
    BreathHungerThirstText->SetPos(10, SpriteSize.y + 8 * VerticalSpacing);
    CurrentTaskText->SetPos(10, SpriteSize.y + 9 * VerticalSpacing);
    WorkPreferenceText->SetPos(g2EntityController_Width / 2 - WorkPreferenceText->GetWidth() / 2, SpriteSize.y + 11 * VerticalSpacing);
    
    int Width = SecondPreferenceGroup->GetWidth();
    SecondPreferenceGroup->SetPos(g2EntityController_Width - 30 - Width, SpriteSize.y + 12 * VerticalSpacing);
    WorkPreferenceGroup->SetPos(30, SpriteSize.y + 12 * VerticalSpacing);
    
    Width = ShowPath->GetLabel()->GetWidth();
    ShowPath->SetPos(g2EntityController_Width / 2 - Width / 2, g2EntityController_Height - 55);
}

DwarfEntity* g2EntityController::GetDwarf()
{
    return (DwarfEntity*)SelectedEntity;
}

void g2EntityController::Render(int x, int y)
{
    // Call standard parent constructor
    g2Panel::Render(x, y);
    
    // All labels are already rendered
    // Explicitly render the preview if available
    if(SelectedEntity != NULL)
    {
        // Get global position
        int gx, gy;
        GetPos(&gx, &gy);
        
        // Get scale and draw
        Vector2<int> PixelSize = SelectedEntity->GetPixelSize();
        SelectedEntity->RenderPreview(gx + g2EntityController_Width / 2 - PixelSize.x / 2, gy + 18, PixelSize.x, PixelSize.y);
    }
}

void g2EntityController::Update(float dT)
{
    // Do not update if no dwarf data
    if(SelectedEntity == NULL)
        return;
    
    // It's a dwarf, so with with a dwarf pointer
    DwarfEntity* Dwarf = (DwarfEntity*)SelectedEntity;
    
    // Place all names / labels
    SetName(Dwarf->GetName());
    SetAgeGender(Dwarf->GetAge(), Dwarf->GetGender());
    SetLevelExp(Dwarf->GetLevel(), Dwarf->GetExp());
    SetRank(DwarfRankNames[Dwarf->GetRank()]);
    SetHealth(Dwarf->GetHealth(), Dwarf->GetMaxHealth());
    SetHappiness(Dwarf->GetHappiness(), Dwarf->GetFatigue());
    SetBreathHungerThirst(Dwarf->GetBreath(), Dwarf->GetMaxBreath(), Dwarf->GetHunger(), Dwarf->GetThirst());
    //SetJobs(Dwarf->GetJob(), Dwarf->GetMinorJob());
    
    // Set the active instruction based on the string explanations
    EntityInstruction Instruction;
    if(Dwarf->GetActiveInstruction(&Instruction))
        SetTask(EntityOpNames[Instruction.Operator]);
    else
        SetTask("No active task");
    
}

void g2EntityController::SetHealth(int current, int max)
{
    char Text[512];
    sprintf(Text, "Health: \\04%d \\00/ \\04%d \\00pts", current, max);
    HealthText->SetText(Text);
}

void g2EntityController::SetHappiness(float Happiness, float Fatigue)
{
    char Text[512];
    sprintf(Text, "Happiness: \\02%.1f \\00/ \\0210\\00, Fatigue: \\02%.1f \\00/ \\0210", Happiness * 10.0f, Fatigue * 10.0f);
    HappinessText->SetText(Text);
}


void g2EntityController::SetBreathHungerThirst(float Breath, float MaxBreath, float Hunger, float Thirst)
{
    char Text[512];
    sprintf(Text, "Breath: \\09%.1f \\00/ \\09%.1f\\00, Hunger: \\02%.1f \\00/ \\0210\\00, Thirst: \\01%.1f \\00/ \\0110", Breath, MaxBreath, Hunger * 10.0f, Thirst * 10.0f);
    BreathHungerThirstText->SetText(Text);
}

void g2EntityController::SetTask(const char* TaskName)
{
    char Text[512];
    sprintf(Text, "Current task: \\09%s", TaskName);
    CurrentTaskText->SetText(Text);
}

void g2EntityController::SetRank(const char* RankName)
{
    char Text[512];
    sprintf(Text, "Colony Rank: \\04%s", RankName);
    RankText->SetText(Text);
}

void g2EntityController::SetLevelExp(int Level, int Exp)
{
    char Text[512];
    sprintf(Text, "Job level: \\04%d\\00 (Exp: \\04%d\\00)", Level, Exp);
    JobText->SetText(Text);
}

void g2EntityController::SetName(const char* Name)
{
    char Text[512];
    sprintf(Text, "\"\\02%s\\00\"", Name);
    NameText->SetText(Text);
}

void g2EntityController::SetAgeGender(int Age, bool IsMale)
{
    char Text[512];
    sprintf(Text, "Age: \"\\04%d\\00\" Gender: \"\\04%s\\00\"", Age, IsMale ? "male" : "female");
    GenderAgeText->SetText(Text);
}

void g2EntityController::SetJobs(DwarfJobs Job, DwarfMinorJobs MinorJob)
{
    WorkPreferenceGroup->SetSelectionIndex((int)Job);
    SecondPreferenceGroup->SetSelectionIndex((int)MinorJob);
}

void g2EntityController::WorkPrefSet(g2Controller* Caller)
{
    // We now that this is a radiogroup, and the parent is *always* this controller
    g2RadioGroup* controller = (g2RadioGroup*)Caller;
    g2EntityController* self = (g2EntityController*)Caller->GetParent();
    
    // Set dwarf's work pref.
    DwarfEntity* Dwarf = self->GetDwarf();
    //Dwarf->SetJob((DwarfJobs)controller->GetSelectionIndex());
}

void g2EntityController::SubWorkPrefSet(g2Controller* Caller)
{
    // We now that this is a radiogroup, and the parent is *always* this controller
    g2RadioGroup* controller = (g2RadioGroup*)Caller;
    g2EntityController* self = (g2EntityController*)Caller->GetParent();
    
    // Set dwarf's work pref.
    DwarfEntity* Dwarf = self->GetDwarf();
    //Dwarf->SetMinorJob((DwarfMinorJobs)controller->GetSelectionIndex());
}

void g2EntityController::PathToggle(g2Controller* Caller)
{
    // We now that this is a radiogroup, and the parent is *always* this controller
    g2CheckBox* controller = (g2CheckBox*)Caller;
    g2EntityController* self = (g2EntityController*)Caller->GetParent();
    
    // Set dwarf's work pref.
    DwarfEntity* Dwarf = self->GetDwarf();
    Dwarf->SetPathRender(controller->IsChecked());
}
