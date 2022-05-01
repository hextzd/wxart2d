/*! \file editor/src/canpropedit.cpp
    \author Klaas Holwerda
    \date Created 11/19/2003

    Copyright: 2003-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: canpropedit.cpp,v 1.13 2008/10/30 23:18:09 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/editor/canpropedit.h"



class PropertyTable : public wxGridTableBase
{
public:
    PropertyTable( a2dEditProperties* editProp ) { m_editProp = editProp; }

    virtual int GetNumberRows();
    virtual int GetNumberCols();
    virtual bool IsEmptyCell( int row, int col );
    virtual wxString GetValue( int row, int col );
    virtual void SetValue( int row, int col, const wxString& value );

    virtual wxString GetColLabelValue( int col );

    virtual wxString GetTypeName( int row, int col );
    virtual bool CanGetValueAs( int row, int col, const wxString& typeName );
    virtual bool CanSetValueAs( int row, int col, const wxString& typeName );

    virtual long GetValueAsLong( int row, int col );
    virtual bool GetValueAsBool( int row, int col );
    virtual double GetValueAsDouble( int row, int col );

    virtual void SetValueAsLong( int row, int col, long value );
    virtual void SetValueAsBool( int row, int col, bool value );
    virtual void SetValueAsDouble( int row, int col, double value );

    bool InsertRows( size_t pos, size_t numRows );
    bool AppendRows( size_t numRows );
    bool DeleteRows( size_t pos, size_t numRows );

public:
    a2dEditProperties* m_editProp;
};

// ----------------------------------------------------------------------------
// property table data
// ----------------------------------------------------------------------------

enum Columns
{
    Col_Name,
    Col_Type,
    Col_Value,
    Col_Max
};

static const wxString PropertyTypeStrs[] =
{
    _T( "string" ),
    _T( "integer" ),
    _T( "real" ),
    _T( "boolean" ),
};

static const wxChar* headers[Col_Max] =
{
    _T( "Name" ),
    _T( "Type" ),
    _T( "Value" ),
};

// ----------------------------------------------------------------------------
// PropertyTable
// ----------------------------------------------------------------------------

wxString PropertyTable::GetTypeName( int row, int col )
{
    switch ( col )
    {
        case Col_Name:
            return wxString::Format( _T( "%s:80" ), wxGRID_VALUE_STRING );

        case Col_Type:
            return wxString::Format( _T( "%s:string,integer,real,boolean" ), wxGRID_VALUE_CHOICE );

        case Col_Value:
            int rowtype = m_editProp->m_propdata [row].m_type;
            switch ( rowtype )
            {
                case pt_string:  return wxGRID_VALUE_STRING;
                case pt_integer: return wxGRID_VALUE_NUMBER;
                case pt_real:    return wxGRID_VALUE_FLOAT;
                case pt_bool:    return wxGRID_VALUE_BOOL;
            }
    }

    wxFAIL_MSG( _T( "unknown column" ) );

    return wxEmptyString;
}

int PropertyTable::GetNumberRows()
{
    return m_editProp->m_propdata.size();
}

int PropertyTable::GetNumberCols()
{
    return Col_Max;
}

bool PropertyTable::IsEmptyCell( int WXUNUSED( row ), int WXUNUSED( col ) )
{
    return false;
}

wxString PropertyTable::GetValue( int row, int col )
{
    const PropGridData& gd = m_editProp->m_propdata[row];

    switch ( col )
    {
        case Col_Name:
            return gd.m_name;

        case Col_Type:
            return PropertyTypeStrs[gd.m_type];

        case Col_Value:
            switch ( gd.m_type )
            {
                case pt_string:  return gd.m_val_str;
                case pt_integer: return wxString::Format( _T( "%d" ), gd.m_val_integer );
                case pt_real:    return wxString::Format( _T( "%f" ), gd.m_val_real );
                case pt_bool:    return gd.m_val_boolean? _T( "1" ) : _T( "0" );
            }
    }

    return wxEmptyString;
}

void PropertyTable::SetValue( int row, int col, const wxString& value )
{
    PropGridData& gd = m_editProp->m_propdata[row];

    switch ( col )
    {
        case Col_Name:
            gd.m_name = value;
            break;

        case Col_Type:
        {
            PropertyType typeNow = gd.m_type;
            size_t n;
            for ( n = 0; n < WXSIZEOF( PropertyTypeStrs ); n++ )
            {
                if ( PropertyTypeStrs[n] == value )
                {
                    gd.m_type = ( PropertyType )n;
                    break;
                }
            }
            if (  typeNow != gd.m_type )
            {
                wxGrid* grid = m_editProp->m_grid;
                switch ( gd.m_type )
                {
                    case pt_string:
                        grid->SetCellEditor( row, Col_Value, new wxGridCellTextEditor() );
                        break;
                    case pt_integer:
                        grid->SetCellEditor( row, Col_Value, new wxGridCellNumberEditor( -1, 1 ) );
                        break;
                    case pt_real:
                        grid->SetCellEditor( row, Col_Value, new wxGridCellFloatEditor( -1, 1 ) );
                        break;
                    case pt_bool:
                        grid->SetCellEditor( row, Col_Value, new wxGridCellBoolEditor() );
                        break;
                }
            }

            if ( n == WXSIZEOF( PropertyTypeStrs ) )
            {
                wxLogWarning( _T( "Invalid type value '%s'." ),
                              value.c_str() );
                gd.m_type = pt_string;
            }
        }
        break;

        case Col_Value:
            switch ( gd.m_type )
            {
                case pt_string: gd.m_val_str = value;
            }
            break;
    }
}

bool
PropertyTable::CanGetValueAs( int row,
                              int col,
                              const wxString& typeName )
{
    int rowtype = m_editProp->m_propdata [row].m_type;

    if ( col == 0 )
    {
        if ( typeName == wxGRID_VALUE_STRING )
            return true;
    }
    else if ( col == 1 )
    {
        if ( typeName == wxGRID_VALUE_NUMBER )
            return true;
        if ( typeName == wxGRID_VALUE_CHOICE )
            return true;
    }
    else if ( col == 2 )
    {
        switch ( rowtype )
        {
            case pt_string:  if ( typeName == wxGRID_VALUE_STRING )
                    return true;
                break;
            case pt_integer: if ( typeName == wxGRID_VALUE_NUMBER )
                    return true;
                break;
            case pt_real:    if ( typeName == wxGRID_VALUE_FLOAT )
                    return true;
                break;
            case pt_bool:    if ( typeName == wxGRID_VALUE_BOOL )
                    return true;
                break;
        }
    }
    return false;
}

bool PropertyTable::CanSetValueAs( int row, int col, const wxString& typeName )
{
    return CanGetValueAs( row, col, typeName );
}

long PropertyTable::GetValueAsLong( int row, int col )
{
    int rowtype = m_editProp->m_propdata [row].m_type;

    if ( col == 2 )
    {
        switch ( rowtype )
        {
            case pt_integer: return m_editProp->m_propdata [row].m_val_integer;
            default:
                wxFAIL_MSG( _T( "not a long in column value" ) );
                return -1;
        }
    }
    else
        wxFAIL_MSG( _T( "unexpected column" ) );
    return -1;
}

bool PropertyTable::GetValueAsBool( int row, int col )
{
    int rowtype = m_editProp->m_propdata [row].m_type;

    if ( col == 2 )
    {
        switch ( rowtype )
        {
            case pt_bool: return m_editProp->m_propdata [row].m_val_boolean;
            default:
                wxFAIL_MSG( _T( "not a bool in column value" ) );
                return false;
        }
    }
    else
        wxFAIL_MSG( _T( "unexpected column" ) );
    return false;
}

double PropertyTable::GetValueAsDouble( int row, int col )
{
    int rowtype = m_editProp->m_propdata [row].m_type;

    if ( col == 2 )
    {
        switch ( rowtype )
        {
            case pt_real: return m_editProp->m_propdata [row].m_val_real;
            default:
                wxFAIL_MSG( _T( "not a real in column value" ) );
        }
    }
    else
        wxFAIL_MSG( _T( "unexpected column" ) );
    return 0.0;
}

void PropertyTable::SetValueAsLong( int row, int col, long value )
{
    int rowtype = m_editProp->m_propdata [row].m_type;

    if ( col == Col_Value )
    {
        switch ( rowtype )
        {
            case pt_integer: m_editProp->m_propdata [row].m_val_integer = value;
                break;
            default:
                wxFAIL_MSG( _T( "unexpected column" ) );
        }
    }
    else
        wxFAIL_MSG( _T( "unexpected column" ) );
}

void PropertyTable::SetValueAsBool( int row, int col, bool value )
{
    int rowtype = m_editProp->m_propdata [row].m_type;

    if ( col == Col_Value )
    {
        switch ( rowtype )
        {
            case pt_bool: m_editProp->m_propdata [row].m_val_boolean = value;
                break;
            default:
                wxFAIL_MSG( _T( "not a boolean in column value" ) );
        }
    }
    else
        wxFAIL_MSG( _T( "unexpected column" ) );
}

void PropertyTable::SetValueAsDouble( int row, int col, double value )
{
    int rowtype = m_editProp->m_propdata [row].m_type;

    if ( col == Col_Value )
    {
        switch ( rowtype )
        {
            case pt_real: m_editProp->m_propdata [row].m_val_real = value;
                break;
            default:
                wxFAIL_MSG( _T( "not a real in column value" ) );
        }
    }
    else
        wxFAIL_MSG( _T( "unexpected column" ) );
}

wxString PropertyTable::GetColLabelValue( int col )
{
    return headers[col];
}

bool PropertyTable::InsertRows( size_t pos, size_t numRows )
{
    wxGrid* grid = m_editProp->m_grid;
    size_t curNumRows = m_editProp->m_propdata.size();

    if ( pos >= curNumRows )
    {
        return AppendRows( numRows );
    }

    m_editProp->m_propdata.resize( curNumRows + numRows );

    int row;
    for ( row = curNumRows + numRows - 1; row > pos + 1; row-- )
    {
        m_editProp->m_propdata [row] = m_editProp->m_propdata [row - 1];
    }
    for ( row = pos + 1; row < pos + 1 + numRows; row++ )
    {
        m_editProp->m_propdata [row].m_name = wxT( "" );
        m_editProp->m_propdata [row].m_type = pt_string;
        m_editProp->m_propdata [row].m_val_str = wxT( "" );
        m_editProp->m_propdata [row].m_val_integer = 0;
        m_editProp->m_propdata [row].m_val_real = 0.0;
        m_editProp->m_propdata [row].m_val_boolean = true;
        m_editProp->m_propdata [row].m_prop = NULL;
    }

    if ( GetView() )
    {
        wxGridTableMessage msg( this,
                                wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                                pos,
                                numRows );

        GetView()->ProcessTableMessage( msg );
    }
    for ( row = pos + 1; row < pos + 1 + numRows; row++ )
    {
        grid->SetCellEditor( row, Col_Value, new wxGridCellTextEditor() );
        if ( !m_editProp->m_propdata [row].m_prop )
            grid->SetReadOnly( row, 1, false );

    }
    return true;
}

bool PropertyTable::AppendRows( size_t numRows )
{
    wxGrid* grid = m_editProp->m_grid;
    size_t curNumRows = m_editProp->m_propdata.size();

    m_editProp->m_propdata.resize( curNumRows + numRows );
    int row;
    for ( row = curNumRows; row < curNumRows + numRows; row++ )
    {
        m_editProp->m_propdata [row].m_name = wxT( "" );
        m_editProp->m_propdata [row].m_type = pt_string;
        m_editProp->m_propdata [row].m_val_str = wxT( "" );
    }

    if ( GetView() )
    {
        wxGridTableMessage msg( this,
                                wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                                numRows );

        GetView()->ProcessTableMessage( msg );
    }
    grid->SetCellEditor( curNumRows + numRows - 1, Col_Value, new wxGridCellTextEditor() );

    return true;
}

bool PropertyTable::DeleteRows( size_t pos, size_t numRows )
{
    size_t curNumRows = m_editProp->m_propdata.size();

    if ( pos >= curNumRows )
    {
        wxFAIL_MSG( wxString::Format
                    (
                        wxT( "Called PropertyTable::DeleteRows(pos=%lu, N=%lu)\nPos value is invalid for present table with %lu rows" ),
                        ( unsigned long )pos,
                        ( unsigned long )numRows,
                        ( unsigned long )curNumRows
                    ) );

        return false;
    }

    if ( numRows > curNumRows - pos )
    {
        numRows = curNumRows - pos;
    }

    if ( numRows >= curNumRows )
    {
        int row;
        for ( row = 0; row < curNumRows; row++ )
        {
            if ( m_editProp->m_propdata [row].m_prop )
                m_editProp->m_propdata [row].m_prop->SetCheck( true );
        }
        m_editProp->m_propdata .clear();
    }
    else
    {
        PropGridDataVec::iterator iterfirst = m_editProp->m_propdata.begin();
        PropGridDataVec::iterator iterlast = m_editProp->m_propdata.begin();
        int row;
        for ( row = 0; row < pos; row++ )
            iterfirst++;
        iterlast = iterfirst;
        for ( row = 0; row < numRows - 1; row++ )
            iterlast++;

        // deleted properties are checked (if already existed)
        for ( row = pos; row < pos + numRows; row++ )
        {
            if ( m_editProp->m_propdata [row].m_prop )
                m_editProp->m_propdata [row].m_prop->SetCheck( true );
        }
        m_editProp->m_propdata.erase( iterfirst, iterlast );
    }

    if ( GetView() )
    {
        wxGridTableMessage msg( this,
                                wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                                pos,
                                numRows );

        GetView()->ProcessTableMessage( msg );
    }

    return true;
}









IMPLEMENT_DYNAMIC_CLASS( a2dEditProperties, wxDialog )

BEGIN_EVENT_TABLE( a2dEditProperties, wxDialog )
    EVT_BUTTON( ID_ADD, a2dEditProperties::OnAddClick )
    EVT_BUTTON( ID_CUT, a2dEditProperties::OnCutClick )\
    EVT_BUTTON( ID_PASTE, a2dEditProperties::OnPasteClick )
    EVT_BUTTON( ID_CANCEL, a2dEditProperties::OnCancelClick )\
    EVT_BUTTON( ID_OKE, a2dEditProperties::OnOkeClick )
    EVT_CLOSE( a2dEditProperties::OnCloseWindow )
    EVT_SIZE( a2dEditProperties::OnSize )
END_EVENT_TABLE()

a2dEditProperties::a2dEditProperties( )
{
}

a2dEditProperties::a2dEditProperties( wxWindow* parent, a2dObject* propobject,
                                      a2dNamedPropertyList* propertylist,
                                      wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_propobject = propobject;
    m_propertylist = propertylist;
    Create( parent, id, caption, pos, size, style );
}

bool a2dEditProperties::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_grid = NULL;
    m_add = NULL;
    m_cut = NULL;
    m_cancel = NULL;
    m_oke = NULL;
    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit( this );
    GetSizer()->SetSizeHints( this );
    Centre();
    return TRUE;
}

void a2dEditProperties::CreateControls()
{
    a2dEditProperties* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer3, 1, wxALIGN_CENTER_HORIZONTAL | wxALL | wxEXPAND, 5 );

    int row = 0;
    a2dNamedPropertyList::const_iterator iter;
    for( iter = m_propertylist->begin(); iter != m_propertylist->end(); ++iter )
    {
        const a2dNamedProperty* prop = *iter;
        if ( prop->GetId()->IsEditable() || prop->GetId()->IsUserDefined() )
            row++;
    }

    m_propdata.resize( row );
    row = 0;
    for( iter = m_propertylist->begin(); iter != m_propertylist->end(); ++iter )
    {
        a2dNamedProperty* prop = *iter;
        if ( prop->GetId()->IsEditable() || prop->GetId()->IsUserDefined() )
        {
            m_propdata [row].m_prop = prop;
            m_propdata [row].m_name = prop->GetName();
            if ( wxDynamicCast(  prop, a2dStringProperty ) )
            {
                m_propdata [row].m_type = pt_string;
                m_propdata [row].m_val_str = prop->GetString();
            }
            else if ( wxDynamicCast(  prop, a2dInt32Property ) )
            {
                m_propdata [row].m_type = pt_integer;
                m_propdata [row].m_val_integer = prop->GetInt32();
            }
            else if ( wxDynamicCast(  prop, a2dDoubleProperty ) )
            {
                m_propdata [row].m_type = pt_real;
                m_propdata [row].m_val_real = prop->GetDouble();
            }
            else if ( wxDynamicCast(  prop, a2dBoolProperty ) )
            {
                m_propdata [row].m_type = pt_bool;
                m_propdata [row].m_val_boolean = prop->GetBool();
            }
            row++;
        }
    }

    m_grid = new wxGrid( itemDialog1, ID_GRID, wxDefaultPosition, wxSize( 370, 150 ), wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL );
    PropertyTable* table = new PropertyTable( this );
    m_grid->SetDefaultColSize( 100 );
    m_grid->SetDefaultRowSize( 25 );
    m_grid->SetColLabelSize( 25 );
    m_grid->SetRowLabelSize( 50 );
    m_grid->SetTable( table, true );
    m_grid->SetColMinimalAcceptableWidth( 50 );

    wxGridCellAttr* attrNameEditor = new wxGridCellAttr,
    *attrTypeEditor = new wxGridCellAttr;

    attrNameEditor->SetEditor( new wxGridCellTextEditor() );
    attrTypeEditor->SetEditor( new wxGridCellChoiceEditor( WXSIZEOF( PropertyTypeStrs ),
                               PropertyTypeStrs ) );
    m_grid->SetColAttr( Col_Name, attrNameEditor );
    m_grid->SetColAttr( Col_Type, attrTypeEditor );

    int j;
    for ( j = 0; j < m_propdata.size(); j++ )
    {
        // if already defined property, do not allow change of type
        int rowtype = m_propdata [j].m_type;
        switch ( rowtype )
        {
            case pt_string:
                m_grid->SetCellEditor( j, 2, new wxGridCellTextEditor() );
                break;
            case pt_integer:
                m_grid->SetCellEditor( j, 2, new wxGridCellNumberEditor( -1, 1 ) );
                break;
            case pt_real:
                m_grid->SetCellEditor( j, 2, new wxGridCellFloatEditor( -1, 1 ) );
                break;
            case pt_bool:
                m_grid->SetCellEditor( j, 2, new wxGridCellBoolEditor() );
                break;
        }
        if ( m_propdata [j].m_prop )
            m_grid->SetReadOnly( j, 1, true );
    }

    itemBoxSizer3->Add( m_grid, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5 );

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 2 );

    m_add = new wxButton( itemDialog1, ID_ADD, _( "Add" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_add->SetDefault();
    m_add->SetHelpText( _( "Add a row for a new property" ) );
    if ( ShowToolTips() )
        m_add->SetToolTip( _( "Add a row for a new property" ) );
    itemBoxSizer5->Add( m_add, 0, wxALIGN_CENTER_VERTICAL | wxALL , 2 );

    m_cut = new wxButton( itemDialog1, ID_CUT, _( "Cut" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_cut->SetHelpText( _( "Remove property at current row" ) );
    if ( ShowToolTips() )
        m_cut->SetToolTip( _( "Remove property at current row" ) );
    itemBoxSizer5->Add( m_cut, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxButton* itemButton8 = new wxButton( itemDialog1, ID_PASTE, _( "Paste" ), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton8->SetHelpText( _( "Paste last cut property" ) );
    if ( ShowToolTips() )
        itemButton8->SetToolTip( _( "Paste last cut property" ) );
    itemBoxSizer5->Add( itemButton8, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 2 );

    m_cancel = new wxButton( itemDialog1, ID_CANCEL, _( "Cancel" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_cancel->SetHelpText( _( "End edit without applying new values and properties" ) );
    if ( ShowToolTips() )
        m_cancel->SetToolTip( _( "End edit without applying new values and properties" ) );
    itemBoxSizer9->Add( m_cancel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

    m_oke = new wxButton( itemDialog1, ID_OKE, _( "Oke" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_oke->SetHelpText( _( "End edit properties" ) );
    if ( ShowToolTips() )
        m_oke->SetToolTip( _( "End edit properties" ) );
    itemBoxSizer9->Add( m_oke, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );
}

bool a2dEditProperties::ShowToolTips()
{
    return TRUE;
}

void a2dEditProperties::OnAddClick( wxCommandEvent& event )
{
    m_grid->InsertRows( m_grid->GetGridCursorRow(), 1 );
}


void a2dEditProperties::OnCutClick( wxCommandEvent& event )
{
    m_grid->DeleteRows( m_grid->GetGridCursorRow(), 1 );
}


void a2dEditProperties::OnPasteClick( wxCommandEvent& event )
{
    event.Skip();
}

void a2dEditProperties::OnCancelClick( wxCommandEvent& event )
{
    EndModal( wxID_CANCEL );
}

void a2dEditProperties::OnOkeClick( wxCommandEvent& event )
{
    int j;
    for ( j = 0; j < m_propdata.size(); j++ )
    {
        a2dNamedProperty* prop = m_propdata [j].m_prop;
        wxString name = m_propdata [j].m_name;
        if ( prop )
        {
            if ( m_propobject->HasPropertyId( prop->GetId() ) )
            {
                bool check = false;
                int rowtype = m_propdata [j].m_type;
                switch ( rowtype )
                {
                    case pt_string:
                    {
                        a2dStringProperty* sprop = wxDynamicCast( prop, a2dStringProperty );
                        if ( sprop )
                            sprop->SetValue( m_propdata [j].m_val_str );
                        check = true;
                        break;
                    }
                    case pt_integer:
                    {
                        a2dInt32Property* dprop = wxDynamicCast( prop, a2dInt32Property );
                        if ( dprop )
                            dprop->SetValue( m_propdata [j].m_val_integer );
                        check = true;
                        break;
                    }
                    case pt_real:
                    {
                        a2dDoubleProperty* dprop = wxDynamicCast( prop, a2dDoubleProperty );
                        if ( dprop )
                            dprop->SetValue( m_propdata [j].m_val_real );
                        check = true;
                        break;
                    }
                    case pt_bool:
                    {
                        a2dBoolProperty* boolprop = wxDynamicCast( prop, a2dBoolProperty );
                        if ( boolprop )
                            boolprop->SetValue( m_propdata [j].m_val_boolean );
                        check = true;
                        break;
                    }
                }
                if ( !check )
                {
                    wxString mes;
                    mes = wxT( "property with name: " ) + name +
                          wxT( "\nhas already bin created on this object with a different type.\n" ) +
                          wxT( "use a different name which does not conflict, or set the type to " ) +
                          PropertyTypeStrs[ rowtype ];
                    wxMessageBox( mes, _( "edit properties" ), wxICON_WARNING | wxOK );
                }
                else
                {
                    // value is already set, change name now.
                    a2dPropertyId* castp = const_cast<a2dPropertyId*>( prop->GetId() );
                    castp->SetName( name );
                }
            }
            else
                wxMessageBox( _( "object is missing a property ID" ), _( "edit properties" ), wxICON_WARNING | wxOK );
        }
        else
        {
            // create a2dPropertyId of the right type, and name, unless it is already available by that name.
            a2dPropertyId* propid = m_propobject->HasPropertyId( name );
            int rowtype = m_propdata [j].m_type;
            if ( propid )
            {
                // a property id with this name is already available, but its type must fit also.
                bool check = false;
                a2dNamedPropertyPtr propcreated = NULL;
                switch ( rowtype )
                {
                    case pt_string:
                    {
                        propcreated = propid->CreatePropertyFromString( m_propdata [j].m_val_str );
                        a2dStringProperty* prop = wxDynamicCast( propcreated.Get(), a2dStringProperty );
                        if ( prop )
                        {
                            check = true;
                            prop->SetValue( m_propdata [j].m_val_str );
                        }
                        break;
                    }
                    case pt_integer:
                    {
                        propcreated = propid->CreatePropertyFromString( wxT( "" ) );
                        a2dInt32Property* prop = wxDynamicCast( propcreated.Get(), a2dInt32Property );
                        if ( prop )
                        {
                            check = true;
                            prop->SetValue( m_propdata [j].m_val_integer );
                        }
                        break;
                    }
                    case pt_real:
                    {
                        propcreated = propid->CreatePropertyFromString( wxT( "" ) );
                        a2dDoubleProperty* prop = wxDynamicCast( propcreated.Get(), a2dDoubleProperty );
                        if ( prop )
                        {
                            check = true;
                            prop->SetValue( m_propdata [j].m_val_real );
                        }
                        break;
                    }
                    case pt_bool:
                    {
                        propcreated = propid->CreatePropertyFromString( wxT( "" ) );
                        a2dBoolProperty* prop = wxDynamicCast( propcreated.Get(), a2dBoolProperty );
                        if ( prop )
                        {
                            check = true;
                            prop->SetValue( m_propdata [j].m_val_boolean );
                        }
                        break;
                    }
                }
                if ( !check )
                {
                    wxString mes;
                    mes = wxT( "property with name: " ) + name +
                          wxT( "\nhas already bin created on this object, but with a different type.\n" ) +
                          wxT( "use a different name which does not conflict, or set the type to " ) +
                          PropertyTypeStrs[ rowtype ];
                    wxMessageBox( mes, _( "edit properties" ), wxICON_WARNING | wxOK );
                }
                else
                    m_propertylist->push_back( propcreated );

            }
            else
            {
                // we need to create a new property ID of the right type
                a2dNamedPropertyPtr propcreated = NULL;
                switch ( rowtype )
                {
                    case pt_string:
                    {
                        propid = new a2dPropertyIdString( name, wxT( "" ), a2dPropertyId::flag_userDefined );
                        m_propobject->AddPropertyId( propid );
                        propcreated = propid->CreatePropertyFromString( m_propdata [j].m_val_str );
                        a2dStringProperty* prop = wxDynamicCast( propcreated.Get(), a2dStringProperty );
                        break;
                    }
                    case pt_integer:
                    {
                        propid = new a2dPropertyIdInt32( name, 0, a2dPropertyId::flag_userDefined );
                        m_propobject->AddPropertyId( propid );
                        propcreated = propid->CreatePropertyFromString( wxT( "" ) );
                        a2dInt32Property* prop = wxDynamicCast( propcreated.Get(), a2dInt32Property );
                        prop->SetValue( m_propdata [j].m_val_integer );
                        break;
                    }
                    case pt_real:
                    {
                        propid = new a2dPropertyIdDouble( name, 0, a2dPropertyId::flag_userDefined );
                        m_propobject->AddPropertyId( propid );
                        propcreated = propid->CreatePropertyFromString( wxT( "" ) );
                        a2dDoubleProperty* prop = wxDynamicCast( propcreated.Get(), a2dDoubleProperty );
                        prop->SetValue( m_propdata [j].m_val_real );
                        break;
                    }
                    case pt_bool:
                    {
                        propid = new a2dPropertyIdBool( name, 0, a2dPropertyId::flag_userDefined );
                        m_propobject->AddPropertyId( propid );
                        propcreated = propid->CreatePropertyFromString( wxT( "" ) );
                        a2dBoolProperty* prop = wxDynamicCast( propcreated.Get(), a2dBoolProperty );
                        prop->SetValue( m_propdata [j].m_val_boolean );
                        break;
                    }
                }
                m_propertylist->push_back( propcreated );
            }
        }
    }

    EndModal( wxID_OK );
}

void a2dEditProperties::OnCloseWindow( wxCloseEvent& event )
{
    EndModal( wxID_CANCEL );
}

void a2dEditProperties::OnSize( wxSizeEvent& event )
{
    event.Skip();
    return;

    int clientw, clienth;
    GetClientSize( &clientw, &clienth );

    GetSizer()->SetSizeHints( this );
    GetSizer()->Fit( this );
    //Layout();
}

























const int    GROUP_BUTTON_OK     = wxID_HIGHEST + 5401 ;
const int    GROUP_BUTTON_CANCEL = wxID_HIGHEST + 5402 ;
const int    GROUP_BUTTON_SHOW   = wxID_HIGHEST + 5403 ;

BEGIN_EVENT_TABLE( a2dPropertyEditorDlg, wxDialog )
    EVT_BUTTON    ( GROUP_BUTTON_OK,  a2dPropertyEditorDlg::CmOk )
    EVT_BUTTON    ( GROUP_BUTTON_CANCEL ,  a2dPropertyEditorDlg::CmCancel )
    EVT_CLOSE     ( a2dPropertyEditorDlg::OnCloseWindow )
END_EVENT_TABLE()

a2dPropertyEditorDlg::a2dPropertyEditorDlg( wxFrame* parent, a2dNamedPropertyList* propertylist ):
    wxDialog( parent, -1, _T( "Property editor" ), wxPoint( 0, 0 ), wxSize( 100, 500 ),
              ( wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxRESIZE_BORDER ), _T( "Property editor" ) )
{
    m_listbox   = new wxListBox( this, -1, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_NEEDED_SB | wxLB_SINGLE );
    m_button1   = new wxButton( this, GROUP_BUTTON_OK,      _T( "Ok" ), wxDefaultPosition, wxSize( 55, 20 ) );
    m_button2   = new wxButton( this, GROUP_BUTTON_CANCEL,  _T( "Cancel" ), wxDefaultPosition, wxSize( 55, 20 ) );

    m_propertylist = NULL;
    m_listbox->Clear();

    wxBoxSizer* item0 = new wxBoxSizer( wxVERTICAL );

    wxStaticText* title = new wxStaticText( this, -1, _T( "properties" ) );
    wxBoxSizer* item3 = new wxBoxSizer( wxHORIZONTAL );
    item3->Add( title, 1, wxALL, 2 );
    item0->Add( item3, 0, wxEXPAND, 2 );
    Centre( wxBOTH );

    wxBoxSizer* item1 = new wxBoxSizer( wxHORIZONTAL );
    item1->Add( m_listbox, 1, wxEXPAND | wxALL, 2 );
    item0->Add( item1, 1, wxEXPAND, 2 );

    wxBoxSizer* item2 = new wxBoxSizer( wxHORIZONTAL );
    item2->Add( m_button1, 1, wxALL, 2 );
    item2->Add( m_button2, 1, wxALL, 2 );

    item0->Add( item2, 0, wxEXPAND, 2 );

    SetAutoLayout( true );
    SetSizer( item0 );
    item0->Fit( this );
    item0->SetSizeHints( this );

    m_listbox->Clear();
    a2dNamedPropertyList::const_iterator iter;
    for( iter = propertylist->begin(); iter != propertylist->end(); ++iter )
    {
        const a2dNamedProperty* prop = *iter;



        m_listbox->Append( prop->GetId()->GetName() );
    }

    if ( !propertylist->empty() )
        m_listbox->SetSelection( 0, true );
}

a2dPropertyEditorDlg::~a2dPropertyEditorDlg()
{
}

void a2dPropertyEditorDlg::CmOk( wxCommandEvent& )
{
    EndModal( wxID_OK );
}

void a2dPropertyEditorDlg::CmCancel( wxCommandEvent& )
{
    EndModal( wxID_CANCEL );
}

void a2dPropertyEditorDlg::OnCloseWindow( wxCloseEvent& WXUNUSED( event ) )
{
    wxCommandEvent eventc;
    CmCancel( eventc );
}
