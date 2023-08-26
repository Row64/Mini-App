#ifndef _BLOCK_PPG_TEMPLATE_CPP
#define _BLOCK_PPG_TEMPLATE_CPP

#include "c_PipelineManager.h"
using namespace AppCore;

namespace Blocks {

    void OpenTestPPG( const string & inName, int inMode ) {

        string ppgName = inName;
        if ( GetPpgIndex( ppgName ) > -1 ) { return; }  // kick out if PPG is already open

        // Set PPG dimensions
        float ppw = 400;
        float pph = 500;
        float ppx = std::max(50.0f, (ADIO->Dim.x - ppw) * 0.5f );
        float ppy = std::max(50.0f, (ADIO->Dim.y - pph) * 0.5f );

        // Add the PPG
        PropertyPage & newPPG = ADIO->MM.AddPPG( ppgName, true, {ppx,ppy}, {ppw,pph} );

        // Specify the Init, Exit, Draw, and Update functions
        newPPG.InitFunction = InitTestPPG; 
        newPPG.ExitFunction = ExitTestPPG;
        newPPG.DrawFunction = DrawTestPPG;
        newPPG.UpdateFunction = UpdateTestPPG;  // (Optional) Function for handling PPG messages

        // Optional: Set a PPG draw mode and reference pane (default is mode_float)
        newPPG.SetMode( inMode );
        newPPG.ModeRefPane = "Overlay";
        if ( PropertyPage::IsUnderlayMode(inMode) ) { newPPG.ShowActionBar = false; }

        // Optional: Set any other PPG variables you want
        // newPPG.Data = "some data string";

    }

    struct TestPPGData {
        // Store any data that needs to persist across frames in here

        bool CheckBox = false;
        bool LastCheckBox = false;

        LabeledTextInput Input;
        string InputText;

        ColorChip CChip;
        int ChipColor;

        vector<string>      ComboOptions = { "Option 1", "Option 2", "Option 3" };
        UIComboBox*         Combo = nullptr;
        int                 LastComboIndex = 0;
        bool*               ComboClicked = nullptr;

        LabeledSliderSet    Sliders;
        float               Width = 100;
        float               Height = 100;

        string              FromDateInput = "";
        string              ToDateInput = "";

    };

    void InitTestPPG(PropertyPage &inPPG) {
        // This is where you build your PPG form

        float esp = 9;
        int ftReg = BaseFont(), ftBold = BaseFontB(), sftBold = GetGeneralFont("tab"), dftReg = GetCodeFont("");

        // Required Setup
        inPPG.DataPtr.push_back( (void *) new TestPPGData() );
        TestPPGData * data = (TestPPGData *) inPPG.DataPtr.back();
        
        UIForm & form = inPPG.MakeNewForm( v_forward );
        form.PadLeft = "10"; form.PadRight = "10"; form.PadTop = "10"; form.PadBottom = "10";

        // Add a horizontal form
        auto sf = form.AddForm( h_forward ); sf->H = "30";

        // Add a text title and check box
        int fontI;
        float fontS;
        GetFontIndAndScale(0, 30, true, false, fontI, fontS);
        auto Title = sf->AddHeader( "Title");
        Title->Format.Font = fontI;
        Title->Format.FontScale = fontS;

        auto cb = sf->AddCheckBox( data->CheckBox, 22 ); 
        cb->MarginTop = cb->MarginBottom = "4";

        sf->AddSpacer(esp);

        auto txt = sf->AddText( "Label Visibility");
        txt->MarginLeft = "10"; 
        txt->Format.Font = ftReg; 
        txt->Alignment = {0, 0.5f};

        // Add Spacer and divider
        form.AddSpacer(esp);
        auto div = form.AddDivider(1); div->Color = 0xCDCDCD;
        form.AddSpacer(esp);

        // Add Example Input Box
        auto ti = data->Input.MakeInput( &form, data->InputText, "Enter Text: " );

        form.AddSpacer(esp);

        // Add Color Chip
        data->CChip.MakeColorChip( &form, &data->ChipColor );

        form.AddSpacer(esp);

        // Add Combo Box
        auto lc = form.AddLabeledComboBox( "TestPPG_Combo_ID", data->ComboOptions, "Options" );
        lc->H = "25";
        lc->Label.MarginRight = "5"; // add a small margin between the label and the combo
        lc->Combo.Expand = 1; // set combo box to expand to width of form
        data->LastComboIndex = lc->Combo.ComboBox.Index = 0; // set initial combo index
        data->Combo = &lc->Combo.ComboBox; // save a pointer to the combo box
        data->ComboClicked = &lc->Combo.Clicked; // save a pointer to whether the combo box was clicked

        form.AddSpacer(esp);

        // Add Sliders

        data->Sliders.MakeSlider( &form, "Width", data->Width, {0,200} );
        data->Sliders.MakeSlider( &form, "Height", data->Height, {0,200} );

        // Add Spacer and divider
        form.AddSpacer(esp);
        div = form.AddDivider(1); div->Color = 0xCDCDCD;
        form.AddSpacer(esp);

        // Add date input
        auto df = form.AddForm( h_forward );
        df->PadLeft = "1"; df->PadRight = "1"; df->PadTop = "1"; df->PadBottom = "1";

        {   // From date input
            auto di = df->AddDateInput( data->FromDateInput, true );
            di->H = "22"; di->W = "150";
            di->UpdateTimeFormat("mm/dd/yyyy hh:MM a");
            GetFontIndAndScale( 0, 18, false, false, di->Default.Font, di->Default.FontScale );                 // Input box font & scale
            GetFontIndAndScale( 0, 18, true, false, di->Calendar.HeaderFont, di->Calendar.HeaderFontScale );    // Month/Year header font & scale
            GetFontIndAndScale( 0, 17, true, false, di->Calendar.DateFont[1], di->Calendar.DateFontScale[1] );  // Selected date font & scale (default font size is 17px)
        }

        auto dt = df->AddText( "to" ); dt->W = "40"; dt->Alignment = {0.5f,0.5f};

        {   // To date input
            auto di = df->AddDateInput( data->ToDateInput, true );
            di->H = "22"; di->W = "150";
            di->UpdateTimeFormat("mm/dd/yyyy hh:MM a");
            GetFontIndAndScale( 0, 18, false, false, di->Default.Font, di->Default.FontScale );                 // Input box font & scale
            GetFontIndAndScale( 0, 18, true, false, di->Calendar.HeaderFont, di->Calendar.HeaderFontScale );    // Month/Year header font & scale
            GetFontIndAndScale( 0, 17, true, false, di->Calendar.DateFont[1], di->Calendar.DateFontScale[1] );  // Selected date font & scale (default font size is 17px)
        }
        
        // Add Spacer and divider
        form.AddSpacer(esp);
        div = form.AddDivider(1); div->Color = 0xCDCDCD;
        form.AddSpacer(esp);

        // Add Paragraph input
        auto p = form.AddParagraphInput( "", false ); p->Expand = 1;

    }

    void DrawTestPPG(PropertyPage &inPPG, float inW, float inH) {
        // This is where you draw & handle events

        float barH = inPPG.ShowActionBar ? 20.0f : 0.0f;

        TestPPGData * data = (TestPPGData *) inPPG.DataPtr.back();

        // Draw the PPG
        inPPG.Form->UpdateDim( inPPG.Dim.x, inPPG.Dim.y-barH);
        inPPG.Form->Draw( inPPG.Pos.x, inPPG.Pos.y+barH, inPPG.Mute );

        if ( inPPG.Form->GetFocus() ) { 
            ADIO->FM.SetFocus( ft_ppg_input,  "TestPPG_"+inPPG.Name );
        } else { 
            ADIO->FM.ClearFocus( ft_ppg_input,  "TestPPG_"+inPPG.Name );
        }

        // Handle PPG Events
        bool HasChange = false;
        if ( data->LastCheckBox != data->CheckBox ) {
            std::cout << "Check box is: " << ( data->CheckBox ? "true" : "false" ) << std::endl;

            // This will broadcasts a message to all the panes in the app. The "ShowOverlayMessage" is only received by the TUI_ChartSheet.
            // If you look in ChartSheet::ListenForMessage( ... ) function definition, you can see it catches the message updates
            // the appropriate ShowOverlayText variable.
            // PMPTR->SendAppMessage( "TUI", "ShowOverlayText", { ( data->CheckBox ? "true" : "false" ) } );
            
            // Update LastCheckBox
            data->LastCheckBox = data->CheckBox;
        }
        else if ( data->Input.CheckEvents( inPPG, HasChange ) ) {
            if ( HasChange ) {
                std::cout << "TestPPG: Input exit focus change: " << data->InputText << std::endl;
            }
        }
        else if ( data->CChip.CheckEvents( inPPG, HasChange, "Test Color Chip" ) ) {
            if ( HasChange ) {
                std::cout << "TestPPG: Color chip change to " << IntToHexColor(data->ChipColor) << std::endl;
            }
        }
        else if ( data->Combo->Index != data->LastComboIndex ) {
            std::cout << "TestPPG: Combo box index changed to " << data->Combo->Index << std::endl;
            data->LastComboIndex = data->Combo->Index;
        }
        else if ( *data->ComboClicked ) {
            std::cout << "TestPPG: Combo box was clicked" << std::endl;
        }
        else if ( data->Sliders.CheckEvents( inPPG, HasChange ) ) {
            if ( HasChange ) {
                std::cout << "TestPPG: Slider " << data->Sliders.ChangeIndex << " was changed" << std::endl;
            }
        }

    }

    void ExitTestPPG(PropertyPage &inPPG) {
        // This is clean up
        inPPG.DeletePPGForm();
        delete reinterpret_cast<TestPPGData*>(inPPG.DataPtr.back());            // Delete the TestPPGData after the form
        inPPG.DataPtr.pop_back();
    }

    int  UpdateTestPPG(PropertyPage &inPPG, string inType, vector<string> inMessage) {
        if ( inType == "Some Type" ) {
            // do stuff
            // return 2;    // 0 = ignored, 1 = received & continue, 2 = received & halt
        }
        return 0;
    }

} // end namespace Blocks

#endif /* _BLOCK_PPG_TEMPLATE_CPP */
