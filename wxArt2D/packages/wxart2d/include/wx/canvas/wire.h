/*! \file wx/canvas/wire.h
    \brief wire classes for connecting objects.

    Classes for auto (re)routing wires (a2dWirePolylineL) are here.

    \author Michael Sögtrop

    Copyright: 2003-2004 (c) Michael Sögtrop

    Licence: wxWidgets Licence

    RCS-ID: $Id: wire.h,v 1.9 2006/12/13 21:43:24 titato Exp $
*/

#ifndef __WXWIRE_H__
#define __WXWIRE_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/artbase/afmatrix.h"
#include "wx/geometry.h"
#include "wx/artbase/bbox.h"

#include "wx/canvas/canobj.h"
#include "wx/canvas/canprim.h"
#include "wx/canvas/polygon.h"
#include "wx/canvas/drawing.h"

class A2DCANVASDLLEXP a2dWirePolylineL;

//! a2dWirePolylineL is a polyline that adjusts itself when the objects it connects move
/*!
    This is done by taking the position of the pins of the connected a2dCanvasObject's,
    and based on that recalculate the position of the polyline points

    \ingroup canvasobject

    \sa a2dWires
*/
class A2DCANVASDLLEXP a2dWirePolylineL: public a2dPolylineL
{
    DECLARE_EVENT_TABLE()
public:

    a2dWirePolylineL();
    a2dWirePolylineL( a2dVertexList* points, bool spline = false  );
    a2dWirePolylineL( const a2dWirePolylineL& poly, CloneOptions options, a2dRefMap* refs );
    ~a2dWirePolylineL();

    void SetConnectionInfo(
        bool inverted,                      //!< The wire is inverted (drawn from end to begin)
        a2dPinClass* objBegin,   //!< The required pin class in the pin of the object at the start of the wire
        a2dPinClass* objEnd      //!< The required pin class in the pin of the object at the end of the wire
    );

    a2dPinClass* GetStartPinClass() const { return m_wireBegin; }
    void SetStartPinClass( a2dPinClass* startPinClass ) { m_wireBegin = startPinClass; }

    a2dPinClass* GetEndPinClass() const { return m_wireEnd; }
    void SetEndPinClass( a2dPinClass* endPinClass ) { m_wireEnd = endPinClass; }

    //! set true result in having straight lines rerouted between begin and end pin
    void SetRouteOneLine( bool oneLine ) { m_oneLine = oneLine; }

    //! see SetRouteOneLine()
    bool GetRouteOneLine() { return m_oneLine; }

    //! indicated if the wire still needs to be rerouted, used in routing algorithms of wire between objects.
    void SetReroute( bool onOff ) { m_reroute = onOff; }

    //! see SetReroute()
    bool GetReroute() const { return m_reroute; }

    //! indicated if the wire is added recently to be rerouted, used in routing algorithms of wire between objects.
    void SetRerouteAdded( bool onOff ) { m_rerouteAdd = onOff; }

    //! see SetRerouteAdded()
    bool GetRerouteAdded() const { return m_rerouteAdd; }

    //! used to set order of rerouting in a set of lines.
    void SetPriority( wxUint8 priority ) { m_priority = priority; }

    //! See SetPriority()
    wxUint8 GetPriority() const { return m_priority; }

    virtual bool IsConnect() const;
    virtual bool NeedsUpdateWhenConnected() const;

    //! adjust pin angles to the last segment at end and begin of wire.
    void AdjustBeginEndPinAngle();

    //! a tool designed for editing a a2dPolylineL, can in principle be used for
    //! a a2dWirePolylineL, if it makes sure that the pins on it, are synchronized with the segments
    //! which change because of the editing. Therefore in a2dPolylineL, at the right points
    //! AdjustAfterChange is called to achieve this.
    virtual bool AdjustAfterChange( bool final );

    //! Split wire at pin given, and add the new split off wire part to parent, creates a new pin at split.
    /*!
        Used also in a2dCommand_SplitAtPin.
        The split of part of this wire, is a cloned version of itself, the same for the split pin.

        \param parent The parent object of this wiure, and new created wire.
        \param splitPin The pin were the split will take place, id not part of the wire yet, it will be added.
        \param splitPinClone The new pin were the split was made, being part of the new wire part
        \param clone New wire part after the split pin.
    */
    a2dPin* SplitAtPin( a2dCanvasObject* parent, a2dPin* splitPin, a2dPin* splitPinClone = NULL, a2dWirePolylineL* clone = NULL, double hitMargin = a2dACCUR );

    //! Join two pins making one pin, and merge the this wire with the parent wire of the other pin
    /*!
        \param parent The parent of this wire, from which connected wire will be removed.
        \param joinPin The pin to join to.
        \param joinPinOther The connected pin to join and remove
    */
    bool JoinAtPin( a2dCanvasObject* parent, a2dPin* joinPin, a2dPin* joinPinOther );

    //! join two wires at joinPin and joinPinOther will be removed.
    /*!
        \param parent The parent of this wire, from which connected wire will be removed.
        \param joinPin The pin to join/remove
        \param joinPinOther The connected pin to join/remove
        \param inFront set true if other wire it's segments wre added in front of the segments of this line.
        \param secondRedirect set true if second wire needed to be reversed in direction
    */
    bool MergAtPin( a2dCanvasObject* parent, a2dPin* joinPin, a2dPin* joinPinOther, bool& inFront, bool& secondRedirect );

    //! make the direction of vertexes the oposite
    void Reverse();

#if wxART2D_USE_CVGIO
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );
    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut& out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
#endif //wxART2D_USE_CVGIO

    //! Set position of an end point and possibly one adjacent point
    void SetEndPoint( int iEnd, int iNext, double x, double y, bool final );

    //! Check if the begin or end pin of the wire is dislocated
    bool IsDislocated();


    //! generates pins on all possible locations where the wire can be connected.
    /*!
        Depending on m_wireBegin and m_wireEnd combined with parameter task, a certain type of a2dPin will be created.
 
        \param pinClass if not NULL, only generate temporary pins that may connect to this pinClass.
        \param task what/how to connect
        \param x can be used to create pins depending on the poition of the mouse inside the object.
        \param y can be used to create pins depending on the poition of the mouse inside the object.
    */
    virtual bool GeneratePinsPossibleConnections( a2dPinClass* pinClass, a2dConnectTask task, double x, double y, double margin = 0 );

	//! if wire has unconnected pins, they will be removed, or zero lenght it will be released.
	//! use commands for undo.
	void OptimizeRerouteWires( a2dCanvasObject* parent, bool removeZero = true, bool allowredirect = true, bool undo = true );

    DECLARE_DYNAMIC_CLASS( a2dWirePolylineL )

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    void OnChar( wxKeyEvent& event );

    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    virtual bool GeneratePins( a2dPinClass* toConnectTo, a2dConnectTask task, double x, double y, double margin = 0 );

    a2dPinClass* m_objBegin;  //!< The pin class in the pin of the object at the start of the wire
    a2dPinClass* m_objEnd;    //!< The pin class in the pin of the object at the end of the wire

    a2dPinClass* m_wireBegin; //!< The pin class in the pin of the wire at the start of the wire
    a2dPinClass* m_wireEnd;   //!< The pin class in the pin of the wire at the end of the wire

    //! This is used temporarily during editing and need not be saved
    bool m_wasVertical: 1;

    bool m_inverted: 1;                     //!< The wire is inverted (drawn from end to begin)

    bool m_oneLine: 1;

    bool m_reroute: 1;

    bool m_rerouteAdd: 1;

    wxUint8 m_priority: 8;

private:
    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dWirePolylineL( const a2dWirePolylineL& other );
};

//! Temporary object used in editing connected objects.
/*! when editing connected objects, the wires connections are cloned and
    shown as edit copies while editing. The objects on the other end of those wires
    copies are terminated with this object.
    This makes it possible to find the original object on that end of the wire if needed.
    Next to that rendering is as a connected pin, while editing.
*/
class A2DCANVASDLLEXP a2dWireEnd: public a2dCanvasObject
{
    DECLARE_DYNAMIC_CLASS( a2dWireEnd )

public:

    a2dWireEnd( double x = 0 , double y = 0 );
    a2dWireEnd( const a2dWireEnd& obj, CloneOptions options, a2dRefMap* refs );

protected:

    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;

private:

    //!this is a not implemented copy constructor that avoids automatic creation of one
    a2dWireEnd( const a2dWireEnd& other );
};


class A2DCANVASDLLEXP a2dCommand_SplitAtPin: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_SplitAtPin() {}

    a2dCommand_SplitAtPin( a2dCanvasObject* parent, a2dPin* splitPin, double hitMargin );

    ~a2dCommand_SplitAtPin( void );

    bool Do();
    bool Undo();
    bool Redo();

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

    a2dCanvasObjectPtr m_parent; 
    a2dPinPtr m_splitPin;
    a2dPinPtr m_splitPinClone;
    a2dSmrtPtr<a2dWirePolylineL> m_wire;
    a2dSmrtPtr<a2dWirePolylineL> m_clone;
    double m_hitMargin;
};

class A2DCANVASDLLEXP a2dCommand_JoinAtPin: public a2dCommand
{
public:
    //!
    /*! \ingroup commandid
    */
    static const a2dCommandId Id;

    a2dCommand_JoinAtPin() {}

    a2dCommand_JoinAtPin( a2dCanvasObject* parent, a2dPin* joinPin, a2dPin* joinPin2 );

    ~a2dCommand_JoinAtPin( void );

    bool Do();
    bool Undo();
    bool Redo();

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

    a2dCanvasObjectPtr m_parent; 
    a2dPinPtr m_joinPin;
    a2dPinPtr m_joinPin2;
    a2dPinPtr m_renamedPin;
    a2dSmrtPtr<a2dWirePolylineL> m_wire;
    a2dSmrtPtr<a2dWirePolylineL> m_wirePin2;
    a2dSmrtPtr<a2dWirePolylineL> m_oldwire;
    bool m_secondRedirect;
    bool m_inFront;
};

class A2DCANVASDLLEXP a2dCommand_EliminateMatrix: public a2dCommand
{
public:

    //! \ingroup commandid
    static const a2dCommandId Id;

    a2dCommand_EliminateMatrix() {}

    a2dCommand_EliminateMatrix( a2dWirePolylineL* wire );

    ~a2dCommand_EliminateMatrix( void );

    bool Do();
    bool Undo();
    bool Redo();

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

    a2dAffineMatrix m_matrix; 
    a2dSmrtPtr<a2dWirePolylineL> m_wire;
};

class A2DCANVASDLLEXP a2dCommand_RemoveZeroWire: public a2dCommand
{
public:

    //! \ingroup commandid
    static const a2dCommandId Id;

    a2dCommand_RemoveZeroWire() {}

    a2dCommand_RemoveZeroWire( a2dCanvasObject* parent, a2dWirePolylineL* wire );

    ~a2dCommand_RemoveZeroWire( void );

    bool Do();
    bool Undo();
    bool Redo();

    inline a2dCanvasCommandProcessor* GetCanvasCmp() { return wxStaticCast( m_cmp, a2dCanvasCommandProcessor ); }

    a2dCanvasObjectPtr m_parent; 
    a2dPinPtr m_joinPin;
    a2dPinPtr m_joinPin2;
    a2dSmrtPtr<a2dWirePolylineL> m_wire;
};

/*!
  A virtual wire is used to make connection between object without really drawing a wire.
  The object makes conenctions based on a label name.
*/
class a2dVirtWire : public a2dText
{
    wxDECLARE_DYNAMIC_CLASS(a2dVirtWire);
public:
    a2dVirtWire( a2dCanvasObject* m_parent = NULL, const wxString& VirtWireName = wxEmptyString, a2dPinClass* pinclass = a2dPinClass::Standard );
    a2dVirtWire( const a2dVirtWire &src, CloneOptions options, a2dRefMap* refs );

    virtual bool IsVirtConnect() const { return true; }

    void SetParent( a2dCanvasObject* parent ) { m_parent = parent; }
    a2dCanvasObject* GetParent() const { return m_parent; }

    void SetVirtWireName( const wxString& VirtWireName ) { m_VirtWireName = VirtWireName; }
    const wxString GetVirtWireName() const { return m_VirtWireName; }

    virtual bool FindConnectedPins( a2dCanvasObjectList& result, a2dPin* pin = NULL, bool walkWires = true, a2dPinClass* searchPinClass = NULL, a2dCanvasObject* isConnectedTo = NULL );

protected:
    virtual a2dObject* DoClone( CloneOptions options, a2dRefMap* refs ) const;
    virtual bool DoStartEdit( wxUint16 editmode, wxEditStyle editstyle );

    virtual void DoSave( wxObject* parent, a2dIOHandlerXmlSerOut &out, a2dXmlSer_flag xmlparts , a2dObjectList* towrite );
    virtual void DoLoad( wxObject* parent, a2dIOHandlerXmlSerIn& parser, a2dXmlSer_flag xmlparts );

    virtual void DoRender( a2dIterC& ic, OVERLAP clipparent );

private:
    a2dVirtWire(const a2dVirtWire &src);

    wxString m_VirtWireName;
    a2dCanvasObject* m_parent; // needed to get to other a2dVirtWire objects, which form the virtual connection
};

/*!
 * a2dVirtWireDlg class declaration
 */

class a2dVirtWireDlg: public wxDialog
{

public:

    wxDECLARE_EVENT_TABLE();

public:

    a2dVirtWireDlg(wxWindow *parent, a2dVirtWire* virtWire );
    virtual ~a2dVirtWireDlg() {}

    void CreateControls();

    a2dVirtWire* GetVirtWire() const { return m_virtWire; }

protected:
    wxBoxSizer *bs;
    wxBoxSizer *bsN;
    wxComboBox *m_CBName;

    a2dVirtWire* m_virtWire;
};


class a2dSingleRoute
{

public:

    a2dSingleRoute( double offset, double smax );
	~a2dSingleRoute(){}

	//! first & last segment min size
	double m_offset;
	//!	object min half size		
	double m_smax;

    double m_p[8][2];
    int    m_pnr;

	/*!
	Computes & corrects the connection path from an object Out pad to an object In
	pad.

	Entry:
		outPad   - the Out pad to be connected
		inPad    - the In pad to be connected
	Exit:
		nump - the number of connection points
		pts  - the connection point list
	*/
	void ComputeCon( a2dPin* outPad, a2dPin* inPad );

	/*!
	Computes & corrects the connection path from an object Out pad to a vertex at a certain angle

	Entry:
		outPad   - the Out pad to be connected
		xw       - x vertex to route to
		yw       - y vertex to route to
        angle    - angle of arrivel

	Exit:
		nump - the number of connection points
		pts  - the connection point list
	*/
    void ComputeConToWireVertex( a2dPin* outPad, double xw, double yw, double angle );

protected:

	/*!
	Computes the connection path from an object Out pad to an object In pad using 
	the orthogonal path algorithm.

	Entry:
		xo   - Out pad x coordinate
		yo   - Out pad y coordinate
		diro - Out pad direction
		xi   - In pad x coordinate
		yi   - In pad y coordinate
		diri - In pad direction
	Exit:
		p - the connection point list
	Return:
		the number of connection points
	*/
	void Connect ( double xo, double yo, int diro, double xi, double yi, int diri );
	void Correct ( double x, double y, double w, double h );
	void Transform ( int mode, int dout, double xout, double yout, double x, double y, double* tx, double* ty );
	void Shape3P ( double xi, double yi );
	void Shape4PD ( double xi, double yi );
	void Shape4PU ( double xi, double yi );
	void Shape5PB ( double	xi, double yi,	int	sign );
	void Shape5PT ( double	xi, double yi,	int	sign );
	void Shape6P ( double	xi, double yi,	int	sign );
	int  Overcross ( double	x, double y, double w, double h,	double	x1, double	y1, double	x2, double	y2 );
	void Optimize ();
};

#endif
