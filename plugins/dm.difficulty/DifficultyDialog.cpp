#include "DifficultyDialog.h"

#include "i18n.h"
#include "iundo.h"
#include "imainframe.h"
#include "iuimanager.h"
#include "iscenegraph.h"

#include "gamelib.h"
#include "registry/registry.h"
#include "string/string.h"

#include <iostream>

#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/button.h>

namespace ui
{

namespace
{
    const char* const WINDOW_TITLE = N_("Difficulty Editor");
}

DifficultyDialog::DifficultyDialog() :
    DialogBase(_(WINDOW_TITLE))
{
    // Load the settings
    _settingsManager.loadSettings();

    // Create the widgets
    populateWindow();
}

void DifficultyDialog::createDifficultyEditors()
{
    int numLevels = game::current::getValue<int>(GKEY_DIFFICULTY_LEVELS);

    for (int i = 0; i < numLevels; i++)
    {
        // Acquire the settings object
        difficulty::DifficultySettingsPtr settings = _settingsManager.getSettings(i);

        if (settings != NULL)
        {
            _editors.push_back(
                DifficultyEditorPtr(new DifficultyEditor(
                    _notebook, _settingsManager.getDifficultyName(i), settings)
                )
            );
        }
    }

    // Pack the editors into the notebook
    for (std::size_t i = 0; i < _editors.size(); i++)
    {
        DifficultyEditor& editor = *_editors[i];

        wxWindow* editorWidget = editor.getEditor();

        // Reparent the DifficultyEditor's widget to the book control and then
        // add it as a new page.
        editorWidget->Reparent(_notebook);
        _notebook->AddPage(editorWidget, editor.getNotebookLabel(), false);
    }
}

void DifficultyDialog::populateWindow()
{
    SetSizer(new wxBoxSizer(wxVERTICAL));

    // Create the notebook and add it to the vbox
    _notebook = new wxChoicebook(this, wxID_ANY);
    _notebook->SetMinClientSize(wxSize(800, 400));

    // Add the edit button alongside the dropdown
    wxSizer* choiceSizer = _notebook->GetControlSizer();
    wxButton* editBtn = new wxButton(
        _notebook, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxBU_EXACTFIT | wxBU_NOTEXT
    );
    editBtn->SetBitmap(wxArtProvider::GetBitmap("darkradiant:edit.png"));
    choiceSizer->Add(editBtn, 0, wxEXPAND);

    // Create and pack the editors
    createDifficultyEditors();

    GetSizer()->Add(_notebook, 1, wxEXPAND | wxALL, 12);

    wxButton* okButton = new wxButton(this, wxID_OK);
    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);

    okButton->Bind(wxEVT_BUTTON, [&] (wxCommandEvent&) { EndModal(wxID_OK); });
    cancelButton->Bind(wxEVT_BUTTON, [&] (wxCommandEvent&) { EndModal(wxID_CANCEL); });

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(cancelButton);
    buttonSizer->AddSpacer(6);
    buttonSizer->Add(okButton);

    GetSizer()->Add(buttonSizer, 0, wxALIGN_RIGHT | wxALL, 12);

    Layout();
    Fit();
}

void DifficultyDialog::save()
{
    // Consistency check can go here

    // Scoped undo object
    UndoableCommand command("editDifficulty");

    // Save the working set to the entity
    _settingsManager.saveSettings();
}

int DifficultyDialog::ShowModal()
{
    int returnCode = DialogBase::ShowModal();

    if (returnCode == wxID_OK)
    {
        save();
    }

    return returnCode;
}

// Static command target
void DifficultyDialog::ShowDialog(const cmd::ArgumentList& args)
{
    // Construct a new instance, this enters the main loop
    DifficultyDialog* editor = new DifficultyDialog;

    editor->ShowModal();
    editor->Destroy();
}

} // namespace ui
