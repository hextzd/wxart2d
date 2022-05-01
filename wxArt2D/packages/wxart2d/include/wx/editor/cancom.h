/*! \file wx/canvas/cancom.h
    \brief a2dCanvasCommandProcessor for do, undo and redo changes in a a2dCanvasDocument

    Each specific Command to change a document is wrapped into class. This class knows what to do,
    in order to make the change to the document. The command procesor recieves such a command, stores
    in the undo stack and tells it to do the command. Undo-ing is reversing the change done,
    which the command class is able to do, since it stores the situation before the change, or it knows
    how to get back in that state.

    \author Klaas Holwerda

    Copyright: 2000-2004 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: cancom.h,v 1.89 2009/10/01 19:22:35 titato Exp $
*/

#ifndef __WXCANCOM_H__
#define __WXCANCOM_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/cmdproc.h"

#include "wx/canvas/canmod.h"
#include "wx/canextobj/imageioh.h"
#include "wx/editor/orderdlg.h"
#include "wx/editor/candoc.h"
#include "wx/docview/doccom.h"
#include <wx/tokenzr.h>
#include <wx/uri.h>
#include <wx/numdlg.h>

#if wxART2D_USE_GDSIO
#include "wx/gdsio/gdsio.h"
#endif //wxART2D_USE_GDSIO

#if wxART2D_USE_KEYIO
#include "wx/keyio/keyio.h"
#endif //wxART2D_USE_KEYIO

class a2dCoordinateEntry;

class A2DEDITORDLLEXP ExecDlg;

class A2DEDITORDLLEXP a2dCentralCanvasCommandProcessor;
class A2DEDITORDLLEXP a2dPropertyEditorDlg;
class A2DEDITORDLLEXP a2dCanvasObjectsDocDialog;

#define  CMDP  (((a2dCanvasDocument*)GetCurrentDocument())->GetCanvasCommandProcessor())

DECLARE_MENU_ITEMID( CmdMenu_GdsIoSaveTextAsPolygon )
DECLARE_MENU_ITEMID( CmdMenu_KeyIoSaveTextAsPolygon )
DECLARE_MENU_ITEMID( CmdMenu_SetGdsIoSaveFromView )
DECLARE_MENU_ITEMID( CmdMenu_SetKeyIoSaveFromView )

DECLARE_MENU_ITEMID( CmdMenu_SaveLayers )
DECLARE_MENU_ITEMID( CmdMenu_SaveLayersHome )
DECLARE_MENU_ITEMID( CmdMenu_SaveLayersLastDir )
DECLARE_MENU_ITEMID( CmdMenu_SaveLayersCurrentDir )
DECLARE_MENU_ITEMID( CmdMenu_LoadLayers )
DECLARE_MENU_ITEMID( CmdMenu_LoadLayersHome )
DECLARE_MENU_ITEMID( CmdMenu_LoadLayersLastDir )
DECLARE_MENU_ITEMID( CmdMenu_LoadLayersCurrentDir )

DECLARE_MENU_ITEMID( CmdMenu_GdsIoSaveFromView )
DECLARE_MENU_ITEMID( CmdMenu_KeyIoSaveFromView )
DECLARE_MENU_ITEMID( CmdMenu_SetLayersProperty )

DECLARE_MENU_ITEMID( CmdMenu_ShowDlgLayers )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgStructureDocument )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgStructureDocumentModal )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgLayersDocument )
DECLARE_MENU_ITEMID( CmdMenu_ShowDlgLayerOrderDocument )

DECLARE_MENU_ITEMID( CmdMenu_ViewAsImage )
DECLARE_MENU_ITEMID( CmdMenu_ViewAsImage_Png )
DECLARE_MENU_ITEMID( CmdMenu_ViewAsImage_Bmp )
DECLARE_MENU_ITEMID( CmdMenu_ViewAsSvg )

DECLARE_MENU_ITEMID( CmdMenu_DocumentAsImage_Png )
DECLARE_MENU_ITEMID( CmdMenu_DocumentAsImage_Bmp )
DECLARE_MENU_ITEMID( CmdMenu_DocumentAsOdbXml )

DECLARE_MENU_ITEMID( CmdMenu_PushTool_Link_OneShot )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Link )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_FollowLinkDocDrawCam )
DECLARE_MENU_ITEMID( CmdMenu_PushTool_Master3Link )

DECLARE_MENU_ITEMID( CmdMenu_CreateViewSelected )


//! a command wrapper specially designed to work with wxArt2D docview classes and rest.
/*!
    This class wraps command string into functions for a a2dDocumentCommandProcessor.

    \ingroup tools commands
*/
class A2DEDITORDLLEXP a2dCentralCanvasCommandProcessor : public a2dDocumentCommandProcessor
{
#if defined(_DEBUG) && defined (SMART_POINTER_DEBUG)
    a2dInitCurrentSmartPointerOwner m_initCurrentSmartPointerOwner;
#endif

    DECLARE_EVENT_TABLE()
    DECLARE_CLASS( a2dCentralCanvasCommandProcessor )

public:

    friend class a2dCommand_GroupAB;

    //! \see a2dDocumentCommandProcessor
    a2dCentralCanvasCommandProcessor( long flags = a2dDEFAULT_DOCUMENT_FLAGS,
                                      bool initialize = true,
                                      int maxCommands = -1 );

    //! initilize with lower level command processor
    a2dCentralCanvasCommandProcessor( a2dDocumentCommandProcessor* other );

    //! destructor
    ~a2dCentralCanvasCommandProcessor();

    void UpdateAllViews();

    void OnUpdateUndo( wxUpdateUIEvent& event );

    void OnUpdateRedo( wxUpdateUIEvent& event );

    //! reroutes events to the current active drawingPart when not handled here.
    /*!
        In general menu commands are handled here or in the a2dDrawingPart.
    */
    bool ProcessEvent( wxEvent& event );

    void OnPushToolUpdateMenu( wxUpdateUIEvent& event );

    void OnUpdateMenu( wxUpdateUIEvent& event );

    //! All menu's a2dMenuIdItem using can be intercepted here
    void OnMenu( wxCommandEvent& event );

    //! Submit a command to the active drawing
    bool SubmitToDrawing( a2dCommand* command, bool storeIt = true );

    //! return current document in the command processor
    a2dCanvasDocument* GetCanvasDocument() { return wxDynamicCast( m_currentDocument, a2dCanvasDocument ); }

    //! Get UnitScale of current document if set else returns 1
    double GetUnitsScaleOfDocument();

    //! grid setting X
    void SetGridX( const a2dDoMu& gridX );
    //! grid setting Y
    void SetGridY( const a2dDoMu& gridY );

    //! refresh what is documents and shown on views
    void Refresh();

    bool SaveLayers( const wxFileName& fileName );

    bool LoadLayers( const wxFileName& fileName );

    //! add the current canvas object to parent as child
    /*! First the current style is to the currentobject, and then it is added to the document.

          \param objectToAdd this is the object to add to the current parentobject
          \param withUndo if true a a2dCommand_AddObject will be used to have undo information
          \param setStyles the object added will get these styles that are currently set in the command processor
          \param parentObject when not NULL, this will become the current parent object.
    */
    a2dCanvasObject* AddCurrent( a2dCanvasObject* objectToAdd, bool withUndo = false, a2dPropertyIdList* setStyles = NULL, a2dCanvasObject* parentObject = NULL );

    bool CvgString( const wxString& cvgString );

    //! To set a property by to a certain object
    /*!
        \param objectname name of the object to set the property to.
            object name is:
            -# a2dBaseTool Sets a property on the a2dToolContr its FirstTool of the current a2dCanvasView.
            -# a2dToolContr Sets a property on the a2dToolContr of the current a2dCanvasView.
            -# a2dCanvasCommandProcessor Sets a property to this a2dCentralCanvasCommandProcessor
            -# a2dCentralCanvasCommandProcessor Sets a property on the current a2dCanvasDocument its wxCommandProcessor
        \param propertyname name of property
        \param value
    */
    bool SetPropertyToObject( const wxString& objectname, const wxString& propertyname, const wxString& value = wxT( "" ) );

    //! To set a property by to a certain object
    /*!
        \param objectname name of the object to set the property to.
            object name is:
            -# a2dBaseTool Sets a property on the a2dToolContr its FirstTool of the current a2dCanvasView.
            -# a2dToolContr Sets a property on the a2dToolContr of the current a2dCanvasView.
            -# a2dCanvasCommandProcessor Sets a property to this a2dCentralCanvasCommandProcessor
            -# a2dCentralCanvasCommandProcessor Sets a property on the current a2dCanvasDocument its wxCommandProcessor
        \param property the property to set
    */
    bool SetPropertyToObject( const wxString& objectname, a2dNamedProperty* property );

    //! set a layer property via command "setlayerproperty"
    /*!
        \param layer layer to add property to
        \param propertyname name of property
        \param value
    */
    bool SetLayerProperty( long layer, const wxString& propertyname, const wxString& value );

    //! returns the current active tool in the current active view
    /*! If the view is of type a2dCanvasView, then if a a2dToolContr is set for it,
        the first tool on its tool stack is returned.
    */
    a2dBaseTool* GetActiveTool();

    //! pop a tool from the tool stack of the current active a2dCanvasView
    /*!

    */
    a2dBaseTool* PopTool();

    //! set show object using index of child
    bool SetShowObject( int index );

    //! zoom into this area
    bool Zoom( double x1, double y1, double x2, double y2, bool upp );

    //! force an update of information on the currently active view
    bool Update();

    //! besides calling the base class, its connect to Current view (active or not) for some events
    void SetCurrentView( a2dView* view );

    //! is a view with a2dCanvasView active
    a2dCanvasView* CheckDrawingView() const;

    //! is a view with a2dCanvasView active and does it have a document
    a2dCanvasDocument* CheckCanvasDocument() const;

    //! get commandprocessor of document
    a2dCanvasCommandProcessor* GetDrawingCmdProcessor()
    {
        a2dDrawingPart* drawer = a2dCanvasGlobals->GetActiveDrawingPart();
        if ( !drawer )
            return NULL;
        if ( !drawer->GetDrawing() )
            return NULL;

        return drawer->GetDrawing()->GetCanvasCommandProcessor();

        //wxASSERT_MSG( CheckDrawingView(), wxT( "current drawing needs to be set" ) );
        //return  CheckDrawingView()->GetDrawingPart()->GetDrawing()->GetCanvasCommandProcessor();
    }

    void FillLibraryPoints();

    a2dCanvasDocument* GetLibraryPoints() const { return m_librarypoints.Get(); }

    //! cleaup modeless dialogs created from here
    virtual void OnExit();

    virtual bool ShowDlg( const a2dCommandId* comID, bool modal, bool onTop );

    void DeleteDlgs();

    bool IsShownStructureDlg();

    //! boolean engine setting
    void SetBooleanEngineMarge( const a2dDoMu& marge );
    //! boolean engine setting
    void SetBooleanEngineGrid( long grid );
    //! boolean engine setting
    void SetBooleanEngineDGrid( double dgrid );
    //! boolean engine setting
    void SetBooleanEngineCorrectionAber( const a2dDoMu& aber ) ;
    //! boolean engine setting
    void SetBooleanEngineCorrectionFactor( const a2dDoMu& aber );
    //! boolean engine setting
    void SetBooleanEngineMaxlinemerge( const a2dDoMu& maxline );
    //! boolean engine setting
    void SetBooleanEngineWindingRule( bool rule );
    //! boolean engine setting
    void SetBooleanEngineRoundfactor( double roundfac ) ;
    void SetBooleanEngineSmoothAber( const a2dDoMu& aber );
    void SetBooleanEngineLinkHoles( bool doLinkHoles );
    //! Polygon/polyline to Arc Minimum radius to test
    void SetRadiusMin( const a2dDoMu& radiusMin );
    //! Polygon/polyline to Arc Maximum radius to test
    void SetRadiusMax( const a2dDoMu& radiusMax ) ;
    //! Polygon/polyline to Arc Maximum abberation
    void SetAberPolyToArc( const a2dDoMu& aber );
    //! Arc to Polygon/polyline Maximum abberation
    void SetAberArcToPoly( const a2dDoMu& aber );
    //! detection of small object, smaller than this
    void SetSmall( const a2dDoMu& smallest );

    bool GdsIoSaveTextAsPolygon( bool onOff );
    bool KeyIoSaveTextAsPolygon( bool onOff );

    bool GdsIISaveFromView( bool onOff );
    bool KeySaveFromView( bool onOff );

    //! Polygon/polyline to Arc Minimum radius to test
    const a2dDoMu&  GetRadiusMin() const ;
    //! Polygon/polyline to Arc Maximum radius to test
    const a2dDoMu& GetRadiusMax() const ;
    //! Polygon/polyline to Arc Maximum abberation
    const a2dDoMu& GetAberPolyToArc() const;
    //! Arc to Polygon/polyline Maximum abberation
    const a2dDoMu& GetAberArcToPoly() const ;
    //! detection of small object, smaller than this
    const a2dDoMu& GetSmall() const;

    //! boolean engine setting
    const a2dDoMu& GetBooleanEngineMarge() const;
    //! boolean engine setting
    long GetBooleanEngineGrid() const;
    //! boolean engine setting
    double GetBooleanEngineDGrid() const;
    //! boolean engine setting
    const a2dDoMu& GetBooleanEngineCorrectionAber() const ;
    //! boolean engine setting
    const a2dDoMu& GetBooleanEngineCorrectionFactor() const;
    //! boolean engine setting
    const a2dDoMu& GetBooleanEngineMaxlinemerge() const ;
    //! boolean engine setting
    bool GetBooleanEngineWindingRule() const ;
    //! boolean engine setting
    double GetBooleanEngineRoundfactor() const ;
    //! boolean engine setting
    const a2dDoMu& GetBooleanEngineSmoothAber() const;
    //! boolean engine setting
    bool GetBooleanEngineLinkHoles() const; 

    //! Save current view as an image of the given type.
    bool SaveViewAsImage( a2dCanvasDocument* doc, const wxString& file, wxBitmapType type, a2dCanvasObject* from );

protected:

/*
    const a2dFill& MX_GetFill() const { return m_currentfill; }
    void MX_SetFill( const a2dFill& value ) { m_currentfill = value; }
    const a2dStroke& MX_GetStroke() const { return m_currentstroke; }
    void MX_SetStroke( const a2dStroke& value ) { m_currentstroke = value; }
    const a2dFont& MX_GetFont() const { return m_font; }
    void MX_SetFont( const a2dFont& value ) { m_font = value; }
*/
    //! do this at command start
    void OnBusyBegin( a2dCommandProcessorEvent& WXUNUSED( event ) );

    //! do this at command end
    void OnBusyEnd( a2dCommandProcessorEvent& WXUNUSED( event ) );

    void OnChangedDocument( a2dCommandEvent& event );

    a2dSmrtPtr<a2dCanvasDocument> m_librarypoints;

    LayerPropertiesDialog* m_layersdlg;

    a2dLayerOrderDialog* m_layerOrderDlg;

    a2dCanvasObjectsDocDialog* m_structDlgDoc;

public:
    // member ids
/*
    static a2dPropertyIdCanvasObject* PROPID_Begin;
    static a2dPropertyIdCanvasObject* PROPID_End;
    static a2dPropertyIdDouble* PROPID_EndScaleX;
    static a2dPropertyIdDouble* PROPID_EndScaleY;
    static a2dPropertyIdBool* PROPID_Spline;

    static a2dPropertyIdFill* PROPID_Fill;
    static a2dPropertyIdStroke* PROPID_Stroke;
    static a2dPropertyIdFont* PROPID_Font;
    static a2dPropertyIdUint16* PROPID_Layer;

    static a2dPropertyIdBool* PROPID_PopBeforePush;
    static a2dPropertyIdBool* PROPID_OneShotTools;
    static a2dPropertyIdBool* PROPID_EditAtEndTools;
    static a2dPropertyIdBool* PROPID_Snap;
*/
    DECLARE_PROPERTIES()
};

A2DCANVASDLLEXP extern a2dDrawer2D* a2dGetDrawer2D();

A2DCANVASDLLEXP extern a2dCentralCanvasCommandProcessor* a2dGetCmdh();



//! set
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_GdsIoSaveTextAsPolygon: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_GdsIoSaveTextAsPolygon )
public:
    static const a2dCommandId Id;

    a2dCommand_GdsIoSaveTextAsPolygon( bool set = true ): a2dCommand( false, Id )
    { m_value = set; }

    virtual a2dObject* Clone( CloneOptions options = clone_deep ) const
    {
        return new a2dCommand_GdsIoSaveTextAsPolygon( m_value );
    }

    bool m_value;

protected:

    bool Do( void )
    {
        const_forEachIn( a2dDocumentTemplateList, &GetCanvasCmp()->GetDocTemplates() )
        {
            a2dDocumentTemplateList::value_type temp = *iter;
            if ( temp->IsVisible() && wxT( "GDS-II" ) == temp->GetDescription() )
            {
#if wxART2D_USE_GDSIO
                wxStaticCast( temp->GetDocumentIOHandlerStrOut(), a2dIOHandlerGDSOut )->SetSaveTextAsVpath( m_value );
#endif //wxART2D_USE_GDSIO
            }
        }
        return true;
    }
    bool Undo( void )
    {
        return false;
    }

    inline a2dCentralCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCentralCanvasCommandProcessor ); }

};

//! set
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_KeyIoSaveTextAsPolygon: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_KeyIoSaveTextAsPolygon )
public:
    static const a2dCommandId Id;

    a2dCommand_KeyIoSaveTextAsPolygon( bool set = true ): a2dCommand( false, Id )
    { m_value = set; }

    virtual a2dObject* Clone( CloneOptions options = clone_deep ) const
    {
        return new a2dCommand_KeyIoSaveTextAsPolygon( m_value );
    }

    bool m_value;

protected:

    bool Do( void )
    {
        const_forEachIn( a2dDocumentTemplateList, &GetCanvasCmp()->GetDocTemplates() )
        {
            a2dDocumentTemplateList::value_type temp = *iter;
            if ( temp->IsVisible() && wxT( "KEY" ) == temp->GetDescription() )
            {
#if wxART2D_USE_KEYIO
                wxStaticCast( temp->GetDocumentIOHandlerStrOut(), a2dIOHandlerKeyOut )->SetSaveTextAsVpath( m_value );
#endif //wxART2D_USE_KEYIO
            }
        }
        return true;
    }
    bool Undo( void )
    {
        return false;
    }

    inline a2dCentralCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCentralCanvasCommandProcessor ); }

};

//! Export view as image
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_ViewAsImage: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_ViewImage )
public:

    //! Export a document to a file
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    class A2DCANVASDLLEXP Args
    {
    public:
        Args()
        {
            argSet.file = false;
            argSet.btype = false;
            argValue.btype = wxBITMAP_TYPE_PNG;
        }

        A2D_ARGUMENT_SETTER( wxFileName, file )
        A2D_ARGUMENT_SETTER( wxBitmapType, btype )

        struct argValue
        {
            wxFileName file;
            wxBitmapType btype;
        } argValue;

        struct argSet
        {
            bool file;
            bool btype;
        } argSet;
    };

    a2dCommand_ViewAsImage( const Args& args = Args() ): a2dCommand( false, Id )
    {
        m_args = args;
        if ( args.argSet.file )
            m_args.file( args.argValue.file );
        else
            throw a2dCommandException( _( "file argument must be set" ) );
    }

    virtual a2dObject* Clone( CloneOptions options = clone_deep ) const
    {
        return new a2dCommand_ViewAsImage( m_args );
    }

    inline a2dCentralCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCentralCanvasCommandProcessor ); }

    Args m_args;

protected:

    virtual bool Do()
    {
        a2dCanvasDocument* doc = GetCanvasCmp()->CheckCanvasDocument();
        if ( !doc )
            return false;
        a2dCanvasView* drawingview = GetCanvasCmp()->CheckDrawingView();
        if ( !drawingview )
            return false;

        a2dPathList path;
        path.Add( wxT( "." ) );
        wxString foundfile = m_args.argValue.file.GetFullPath();
        if ( !path.ExpandPath( foundfile ) )
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_CouldNotEvaluatePath, _( "Could not expand %s resulted in %s" ), m_args.argValue.file.GetFullPath().c_str(), foundfile.c_str() );
            return false;
        }
        if ( foundfile.IsEmpty() )
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_NotSpecified, _( "Filename %s not in %s" ), foundfile.c_str(), path.GetAsString().c_str() );
            return false;
        }

        drawingview->GetDrawingPart()->GetDrawer2D()->GetBuffer().SaveFile( foundfile, m_args.argValue.btype, ( wxPalette* )NULL );

        return true;
    }

    virtual bool Undo()
    {
        return false;
    }
};

//! Export view as ViewAsSvg
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_ViewAsSvg: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_ViewAsSvg )
public:

    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    class A2DCANVASDLLEXP Args
    {
    public:
        Args()
        {
            argSet.file = false;
        }

        A2D_ARGUMENT_SETTER( wxFileName, file )

        struct argValue
        {
            wxFileName file;
        } argValue;

        struct argSet
        {
            bool file;
        } argSet;
    };

    a2dCommand_ViewAsSvg( const Args& args = Args() ): a2dCommand( false, Id )
    {
        m_args = args;

        if ( args.argSet.file )
            m_args.file( args.argValue.file );
        else
            throw a2dCommandException( _( "file argument must be set" ) );
    }

    virtual a2dObject* Clone( CloneOptions options = clone_deep ) const
    {
        return new a2dCommand_ViewAsSvg( m_args );
    }

    inline a2dCentralCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCentralCanvasCommandProcessor ); }

    Args m_args;

private:

    virtual bool Do()
    {
        a2dCanvasDocument* doc = GetCanvasCmp()->CheckCanvasDocument();
        if ( !doc )
            return false;
        a2dCanvasView* drawingview = GetCanvasCmp()->CheckDrawingView();
        if ( !drawingview )
            return false;

        a2dPathList path;
        path.Add( wxT( "." ) );
        wxString foundfile = m_args.argValue.file.GetFullPath();
        if ( !path.ExpandPath( foundfile ) )
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_CouldNotEvaluatePath, _( "Could not expand %s resulted in %s" ), m_args.argValue.file.GetFullPath().c_str(), foundfile.c_str() );
            return false;
        }
        if ( foundfile.IsEmpty() )
        {
            a2dDocviewGlobals->ReportErrorF( a2dError_NotSpecified, _( "Filename %s not in %s" ), foundfile.c_str(), path.GetAsString().c_str() );
            return false;
        }

        doc->WriteSVG( drawingview, foundfile, drawingview->GetDrawingPart()->GetShowObject(), 10, 10, _T( "cm" ) );

        return true;
    }

    virtual bool Undo()
    {
        return false;
    }
};

//! set
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_KeyIoSaveFromView: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_KeyIoSaveTextAsPolygon )
public:
    static const a2dCommandId Id;

    a2dCommand_KeyIoSaveFromView( bool set = true ): a2dCommand( false, Id )
    { m_value = set; }

    virtual a2dObject* Clone( CloneOptions options = clone_deep ) const
    {
        return new a2dCommand_KeyIoSaveFromView( m_value );
    }

    bool m_value;

protected:

    bool Do( void )
    {
        const_forEachIn( a2dDocumentTemplateList, &GetCanvasCmp()->GetDocTemplates() )
        {
            a2dDocumentTemplateList::value_type temp = *iter;
            if ( temp->IsVisible() && wxT( "KEY" ) == temp->GetDescription() )
            {
#if wxART2D_USE_KEYIO
                a2dIOHandlerKeyOut* handler = wxStaticCast( temp->GetDocumentIOHandlerStrOut(), a2dIOHandlerKeyOut );
                bool currentSetting = handler->GetFromViewAsTop();
                handler->SetFromViewAsTop( m_value );
                m_value = currentSetting;
#endif //wxART2D_USE_KEYIO
            }
        }
        return true;
    }
    bool Undo( void )
    {
        return Do();
    }

    inline a2dCentralCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCentralCanvasCommandProcessor ); }

};

//! set
/*!
    \ingroup commands
*/
class A2DCANVASDLLEXP a2dCommand_GdsIoSaveFromView: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_GdsIoSaveFromView )
public:
    static const a2dCommandId Id;

    a2dCommand_GdsIoSaveFromView( bool set = true ): a2dCommand( false, Id )
    { m_value = set; }

    virtual a2dObject* Clone( CloneOptions options = clone_deep ) const
    {
        return new a2dCommand_GdsIoSaveFromView( m_value );
    }

    bool m_value;

protected:

    bool Do( void )
    {
        const_forEachIn( a2dDocumentTemplateList, &GetCanvasCmp()->GetDocTemplates() )
        {
            a2dDocumentTemplateList::value_type temp = *iter;
            if ( temp->IsVisible() && wxT( "GDS-II" ) == temp->GetDescription() )
            {
#if wxART2D_USE_GDSIO
                a2dIOHandlerGDSOut* handler = wxStaticCast( temp->GetDocumentIOHandlerStrOut(), a2dIOHandlerGDSOut );
                bool currentSetting = handler->GetFromViewAsTop();
                handler->SetFromViewAsTop( m_value );
                m_value = currentSetting;
#endif //wxART2D_USE_GDSIO
            }
        }
        return true;
    }
    bool Undo( void )
    {
        return Do();
    }

    inline a2dCentralCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCentralCanvasCommandProcessor ); }

};

//! command to show a dialog
/*!
    \ingroup commands
*/
class A2DEDITORDLLEXP a2dCommand_ShowDlg: public a2dCommand
{
    DECLARE_DYNAMIC_CLASS( a2dCommand_ShowDlg )

public:

    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    class A2DEDITORDLLEXP Args
    {
    public:
        Args()
        {
            argSet.which = false;
            argSet.modal = false;
            argValue.modal = false;
            argValue.onTop = false;
        }

        Args& which( const a2dCommandId& arg )
        {
            this->argValue.which = &arg;
            this->argSet.which = true;
            return *this;
        }

        A2D_ARGUMENT_SETTER( bool, modal )
        A2D_ARGUMENT_SETTER( bool, onTop )

        struct argValue
        {
            const a2dCommandId* which;
            bool modal;
            bool onTop;
        } argValue;

        struct argSet
        {
            bool which;
            bool modal;
            bool onTop;
        } argSet;
    };

    a2dCommand_ShowDlg( const a2dCommandId& which = a2dCanvasCommandProcessor::COMID_ShowDlgStyle, bool modal = false, bool onTop = true ): a2dCommand( false, Id )
    {
        m_args.argSet.which = true;
        m_args.argValue.which = &which;
        m_args.argSet.modal = true;
        m_args.argValue.modal = modal;
        m_args.argValue.onTop = onTop;
    }

    a2dCommand_ShowDlg( const Args& args ): a2dCommand( false, Id )
    {
        m_args = args;
    }

    virtual a2dObject* Clone( CloneOptions options = clone_deep ) const
    {
        return new a2dCommand_ShowDlg( m_args );
    }

    inline a2dCentralCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCentralCanvasCommandProcessor ); }

private:

    Args m_args;

    virtual bool Do()
    {
        return GetCanvasCmp()->ShowDlg( m_args.argValue.which, m_args.argValue.modal, m_args.argValue.onTop );
    }

    virtual bool Undo()
    {
        return false;
    }
};

#endif

