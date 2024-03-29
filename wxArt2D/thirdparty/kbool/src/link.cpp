/*! \file src/link.cpp
    \author Klaas Holwerda
 
    Copyright: 2001-2004 (C) Klaas Holwerda
 
    Licence: see kboollicense.txt 
 
    RCS-ID: $Id: link.cpp,v 1.4 2009/09/07 19:23:28 titato Exp $
*/

#include "kbool/booleng.h"

#include "kbool/link.h"
#include "kbool/line.h"
#include <math.h>
#include <assert.h>

#include "kbool/node.h"
#include "kbool/graph.h"
#include "kbool/graphlst.h"

int linkXYsorter( kbLink *, kbLink * );

//
// Default constructor
//
kbLink::kbLink( kbBool_Engine* GC )
{
    _GC = GC;
    m_arcPiece = false;
    Reset();
    linecrosslist = NULL;
}


//
// This constructor makes this link a valid part of a graph
//
kbLink::kbLink( int graphnr, kbNode *begin, kbNode *end, kbBool_Engine* GC )
{
    _GC = GC;
    m_arcPiece = false;
    Reset();

    // Set the references of the node and of this link correct
    begin->AddLink( this );
    end->AddLink( this );
    m_beginnode = begin;
    m_endnode = end;
    m_graphnum = graphnr;
    m_bin = m_hole = false;
    linecrosslist = NULL;
}

//
// This constructor makes this link a valid part of a graph
//
kbLink::kbLink( kbNode *begin, kbNode *end, kbBool_Engine* GC )
{
    _GC = GC;
    m_arcPiece = false;
    Reset();

    // Set the references of the node and of this link correct
    begin->AddLink( this );
    end->AddLink( this );
    m_beginnode = begin;
    m_endnode = end;
    m_graphnum = 0;
    m_hole = false;
    linecrosslist = NULL;
}


//
// Destructor
//
kbLink::~kbLink()
{
    UnLink();
    if ( linecrosslist )
        delete linecrosslist;
}

//
// Checks whether the current algorithm has been on this link
//
bool kbLink::BeenHere()
{
    if ( m_bin ) return true;
    return false;
}

void kbLink::TakeOverOperationFlags( kbLink* link )
{
    m_merge_L = link->m_merge_L;
    m_a_substract_b_L = link->m_a_substract_b_L;
    m_b_substract_a_L = link->m_b_substract_a_L;
    m_intersect_L = link->m_intersect_L;
    m_exor_L = link->m_exor_L;

    m_merge_R = link->m_merge_R;
    m_a_substract_b_R = link->m_a_substract_b_R;
    m_b_substract_a_R = link->m_b_substract_a_R;
    m_intersect_R = link->m_intersect_R;
    m_exor_R = link->m_exor_R;
}
//
// Returns the next link from the argument
//
kbLink* kbLink::Forth( kbNode *node )
{
    assert( node == m_beginnode || node == m_endnode );
    return node->GetOtherLink( this );
}

//
// Returns the Beginnode
//
kbNode *kbLink::GetBeginNode()
{
    return m_beginnode;
}

//
// Returns the endnode
//
kbNode* kbLink::GetEndNode()
{
    return m_endnode;
}

kbNode* kbLink::GetLowNode()
{
    return ( ( m_beginnode->GetY() < m_endnode->GetY() ) ? m_beginnode : m_endnode );
}

kbNode* kbLink::GetHighNode()
{
    return ( ( m_beginnode->GetY() > m_endnode->GetY() ) ? m_beginnode : m_endnode );
}

//
// Returns the graphnumber
//
int kbLink::GetGraphNum()
{
    return m_graphnum;
}

bool kbLink::GetInc()
{
    return m_Inc;
//   if (Inc) return true;
// return false;
}

void kbLink::SetInc( bool inc )
{
    m_Inc = inc;
//   Inc=0;
//   if (inc) Inc=1;
}

bool kbLink::GetLeftA()
{
    return m_LeftA;
}

void kbLink::SetLeftA( bool la )
{
    m_LeftA = la;
}

bool kbLink::GetLeftB()
{
    return m_LeftB;
}

void kbLink::SetLeftB( bool lb )
{
    m_LeftB = lb;
}

bool kbLink::GetRightA()
{
    return m_RightA;
}

void kbLink::SetRightA( bool ra )
{
    m_RightA = ra;
}

bool kbLink::GetRightB()
{
    return m_RightB;
}

void kbLink::SetRightB( bool rb )
{
    m_RightB = rb;
}

bool kbLink::GetArcPiece()
{
    return m_arcPiece;
}

void kbLink::SetArcPiece( bool arcPiece )
{
    m_arcPiece = arcPiece;
}

//
// This function is very popular by GP-faults
// It returns the node different from a
//
kbNode* kbLink::GetOther( const kbNode *const a )
{
    return ( ( a != m_beginnode ) ? m_beginnode : m_endnode );
}


//
// Is this marked for given operation
//
bool kbLink::IsMarked( BOOL_OP operation )
{
    switch ( operation )
    {
        case( BOOL_OR ):     return m_merge_L || m_merge_R;
        case( BOOL_AND ):    return m_intersect_L || m_intersect_R;
        case( BOOL_A_SUB_B ):  return m_a_substract_b_L || m_a_substract_b_R;
        case( BOOL_B_SUB_A ):  return m_b_substract_a_L || m_b_substract_a_R;
        case( BOOL_EXOR ):    return m_exor_L || m_exor_R;
        default:             return false;
    }
}

bool kbLink::IsMarkedLeft( BOOL_OP operation )
{
    switch ( operation )
    {
        case( BOOL_OR ):      return m_merge_L;
        case( BOOL_AND ):     return m_intersect_L;
        case( BOOL_A_SUB_B ): return m_a_substract_b_L;
        case( BOOL_B_SUB_A ): return m_b_substract_a_L;
        case( BOOL_EXOR ):    return m_exor_L;
        default:            return false;
    }
}

bool kbLink::IsMarkedRight( BOOL_OP operation )
{
    switch ( operation )
    {
        case( BOOL_OR ):      return m_merge_R;
        case( BOOL_AND ):     return m_intersect_R;
        case( BOOL_A_SUB_B ): return m_a_substract_b_R;
        case( BOOL_B_SUB_A ): return m_b_substract_a_R;
        case( BOOL_EXOR ):    return m_exor_R;
        default:            return false;
    }
}

//
// Is this a hole for given operation
// beginnode must be to the left
bool kbLink::IsHole( BOOL_OP operation )
{

    bool topsideA, topsideB;

    if ( m_beginnode->GetX() < m_endnode->GetX() ) //going to the right?
    {  topsideA = m_RightA; topsideB = m_RightB;  }
    else
    {  topsideA = m_LeftA; topsideB = m_LeftB; }

    switch ( operation )
    {
        case( BOOL_OR ):      return ( !topsideB && !topsideA );
        case( BOOL_AND ):     return ( !topsideB || !topsideA );
        case( BOOL_A_SUB_B ): return ( topsideB || !topsideA );
        case( BOOL_B_SUB_A ): return ( topsideA || !topsideB );
        case( BOOL_EXOR ):    return !( ( topsideB && !topsideA ) || ( !topsideB && topsideA ) );
        default:            return false;
    }
}

//
// Is this a part of a hole
//
bool kbLink::GetHole()
{
    return ( m_hole );
}


void kbLink::SetHole( bool h )
{
    m_hole = h;
}


//
// Is this not marked at all
//
bool kbLink::IsUnused()
{
    return
        !( m_merge_L || m_merge_R ||
           m_a_substract_b_L || m_a_substract_b_R ||
           m_b_substract_a_L || m_b_substract_a_R ||
           m_intersect_L || m_intersect_R ||
           m_exor_L || m_exor_R );
}


bool kbLink::IsZero( B_INT marge )
{
    return ( m_beginnode->Equal( m_endnode, marge ) ) ;
}


bool kbLink::ShorterThan( B_INT marge )
{
    return ( m_beginnode->ShorterThan( m_endnode, marge ) ) ;
}


//
// Mark this link
//
void kbLink::Mark()
{
    m_mark = true;
}


#ifndef ABS
#define ABS(a) (((a)<0) ? -(a) : (a))
#endif


//
// This makes from the begin and endnode one node (argument begin_or_end_node)
// The references to this link in the node will also be deleted
// After doing that, link link can be deleted or be recycled.
//
void kbLink::MergeNodes( kbNode *const begin_or_end_node )
{
// assert(beginnode && endnode);
// assert ((begin_or_end_node == beginnode)||(begin_or_end_node == endnode));

    m_beginnode->RemoveLink( this );
    m_endnode->RemoveLink( this );

    if ( m_endnode != m_beginnode )
    { // only if beginnode and endnode are different nodes
        begin_or_end_node->Merge( GetOther( begin_or_end_node ) );
    }
    m_endnode = NULL;
    m_beginnode = NULL;
}

//
// Return the position of the second link compared to this link
// Result = IS_ON | IS_LEFT | IS_RIGHT
// Here Left and Right is defined as being left or right from
// the this link towards the center (common) node
//
LinkStatus kbLink::OutProduct( kbLink* const two, double accur )
{
    kbNode * center;
    double distance;
    if ( two->GetBeginNode()->Equal( two->GetEndNode(), 1 ) )
        assert( !two );
    if ( GetBeginNode()->Equal( GetEndNode(), 1 ) )
        assert( !this );
    kbLine* temp_line = new kbLine( this, _GC );

    //the this link should connect to the other two link at at least one node
    if ( m_endnode == two->m_endnode || m_endnode == two->m_beginnode )
        center = m_endnode;
    else
    {
        center = m_beginnode;
//  assert(center==two->endnode || center==two->beginnode);
    }

    //here something tricky
    // the factor 10000.0 is needed to asure that the pointonline
    // is more accurate in this case compared to the intersection for graphs
    int uitp = temp_line->PointOnLine( two->GetOther( center ), distance, accur );

    delete temp_line;

    /*double uitp= (_x - first._x) * (third._y - _y) -
        (_y - first._y) * (third._x - _x);
    if (uitp>0) return IS_LEFT;
    if (uitp<0) return IS_RIGHT;
    return IS_ON;*/

    //depending on direction of this link (going to or coming from centre)
    if ( center == m_endnode )
    {
        if ( uitp == LEFT_SIDE )
            return IS_LEFT;
        if ( uitp == RIGHT_SIDE )
            return IS_RIGHT;
    }
    else  //center=beginnode
    {
        if ( uitp == LEFT_SIDE )
            return IS_RIGHT;
        if ( uitp == RIGHT_SIDE )
            return IS_LEFT;
    }
    return IS_ON;
}

//
// Return the position of the third link compared to this link and
// the second link
// Result = IS_ON | IS_LEFT | IS_RIGHT
//
LinkStatus kbLink::PointOnCorner( kbLink* const two, kbLink* const third )
{
    LinkStatus
    TwoToOne,  // Position of two to this line
    ThirdToOne,    // Position of third to this line
    ThirdToTwo,  // Position of third to two
    Result;

//m  kbNode* center;

//the this link should connect to the other two link at at least one node
//m  if (endnode==two->endnode || endnode==two->beginnode)
//m   center=endnode;
//m  else
//m  { center=beginnode;
//  assert(center==two->endnode || center==two->beginnode);
//m }
// assert(center==third->endnode || center==third->beginnode);



    // Calculate the position of the links compared to eachother
    TwoToOne  = OutProduct( two, _GC->GetAccur() );
    ThirdToOne = OutProduct( third, _GC->GetAccur() );
    //center is used in outproduct to give de direction of two
    // this is why the result should be swapped
    ThirdToTwo = two->OutProduct( third, _GC->GetAccur() );
    if ( ThirdToTwo == IS_RIGHT )
        ThirdToTwo = IS_LEFT;
    else if ( ThirdToTwo == IS_LEFT )
        ThirdToTwo = IS_RIGHT;

    // Select the result
    switch( TwoToOne )
    {
            // Line 2 lies on  leftside of this line
        case IS_LEFT : if ( ( ThirdToOne == IS_RIGHT ) || ( ThirdToTwo == IS_RIGHT ) ) return IS_RIGHT;
            else if ( ( ThirdToOne == IS_LEFT ) && ( ThirdToTwo == IS_LEFT ) ) return IS_LEFT;
            else Result = IS_ON; break;
            // Line 2 lies on this line
        case IS_ON  : if ( ( ThirdToOne == IS_RIGHT ) && ( ThirdToTwo == IS_RIGHT ) )    return IS_RIGHT;
            else if ( ( ThirdToOne == IS_LEFT ) && ( ThirdToTwo == IS_LEFT ) )   return IS_LEFT;
            //  else if ((ThirdToOne==IS_RIGHT) && (ThirdToTwo==IS_LEFT))   return IS_RIGHT;
            //  else if ((ThirdToOne==IS_LEFT) && (ThirdToTwo==IS_RIGHT))   return IS_LEFT;
            else Result = IS_ON; break;
            // Line 2 lies on right side of this line
        case IS_RIGHT : if ( ( ThirdToOne == IS_RIGHT ) && ( ThirdToTwo == IS_RIGHT ) ) return IS_RIGHT;
            else if ( ( ThirdToOne == IS_LEFT ) || ( ThirdToTwo == IS_LEFT ) ) return IS_LEFT;
            else Result = IS_ON; break;
        default: Result = IS_ON; assert( false );
    }
    return Result;
}

//
// Remove the reference from this link to a_node
//
void kbLink::Remove( kbNode *a_node )
{
    ( m_beginnode == a_node ) ? m_beginnode = NULL : m_endnode = NULL;
}


//
// Replace oldnode by newnode and correct the references
//
void kbLink::Replace( kbNode *oldnode, kbNode *newnode )
{
    if ( m_beginnode == oldnode )
    {
        m_beginnode->RemoveLink( this ); // remove the reference to this
        newnode->AddLink( this );       // let newnode refer to this
        m_beginnode = newnode;    // let this refer to newnode
    }
    else
    { //assert(endnode==oldnode);
        m_endnode->RemoveLink( this );
        newnode->AddLink( this );
        m_endnode = newnode;
    }
}


//
// Reset all values
//
void kbLink::Reset()
{
    m_beginnode = 0;
    m_endnode = 0;
    Reset_flags();
}


//
// Reset all flags
//
void kbLink::Reset_flags()
{
    m_bin = false;    // Marker for walking over the graph
    m_hole  = false;   // Is this a part of hole ?
    m_hole_top = false;    // link that is toplink of hole?
    m_group = GROUP_A;  // Does this belong to group A or B ( o.a. for boolean operations between graphs)
    m_LeftA = false;      // Is left in polygongroup A
    m_RightA = false;      // Is right in polygon group A
    m_LeftB = false;      // Is left in polygon group B
    m_RightB = false;      // Is right in polygongroup B
    m_mark = false;      // General purose marker, internally unused
    m_holelink = false;

    m_merge_L = m_merge_R = false;   // Marker for Merge
    m_a_substract_b_L = m_a_substract_b_R = false; // Marker for substract
    m_b_substract_a_L = m_b_substract_a_R = false; // Marker for substract
    m_intersect_L = m_intersect_R = false;  // Marker for intersect
    m_exor_L = m_exor_R = false;          // Marker for Exor
}

//
// Refill this link by the arguments
//
void kbLink::Reset( kbNode *begin, kbNode *end, int graphnr )
{
    // Remove all the previous references
    UnLink();
    Reset();
    // Set the references of the node and of this link correct
    begin->AddLink( this );
    end->AddLink( this );
    m_beginnode = begin;
    m_endnode = end;
    if ( graphnr != 0 )
        m_graphnum = graphnr;
}


void kbLink::Set( kbNode *begin, kbNode *end )
{
    m_beginnode = begin;
    m_endnode = end;
}

void kbLink::SetBeenHere()
{
    m_bin = true;
}

void kbLink::SetNotBeenHere()
{
    m_bin = false;
}

void kbLink::SetBeginNode( kbNode* new_node )
{
    m_beginnode = new_node;
}


void kbLink::SetEndNode( kbNode* new_node )
{
    m_endnode = new_node;
}


//
// Sets the graphnumber to argument num
//
void kbLink::SetGraphNum( int num )
{
    m_graphnum = num;
}

GroupType kbLink::Group()
{
    return m_group;
}


//
// Reset the groupflag to argument groep
//
void kbLink::SetGroup( GroupType groep )
{
    m_group = groep;
}


//
// Remove all references to this link and from this link
//
void kbLink::UnLink()
{
    if ( m_beginnode )
    {
        m_beginnode->RemoveLink( this );
        if ( !m_beginnode->GetNumberOfLinks() ) delete m_beginnode;
    }
    m_beginnode = NULL;
    if ( m_endnode )
    {
        m_endnode->RemoveLink( this );
        if ( !m_endnode->GetNumberOfLinks() ) delete m_endnode;
    }
    m_endnode = NULL;
}


void kbLink::UnMark()
{
    m_mark = false;
    m_bin = false;
}

void kbLink::SetMark( bool value )
{
    m_mark = value;
}

//
// general purpose mark checker
//
bool kbLink::IsMarked() { return m_mark; }

void  kbLink::SetTopHole( bool value ) { m_hole_top = value; }

bool kbLink::IsTopHole() { return m_hole_top; }

//
// Calculates the merge/substact/exor/intersect flags
//
void kbLink::SetLineTypes()
{
    m_merge_R     =
        m_a_substract_b_R =
            m_b_substract_a_R =
                m_intersect_R =
                    m_exor_R      =
                        m_merge_L     =
                            m_a_substract_b_L =
                                m_b_substract_a_L =
                                    m_intersect_L =
                                        m_exor_L      = false;

    //if left side is in group A and B then it is for the merge
    m_merge_L   = m_LeftA || m_LeftB;
    m_merge_R   = m_RightA || m_RightB;
    //both in mean does not add to result.
    if ( m_merge_L && m_merge_R )
        m_merge_L = m_merge_R = false;

    m_a_substract_b_L = m_LeftA && !m_LeftB;
    m_a_substract_b_R = m_RightA && !m_RightB;
    //both in mean does not add to result.
    if ( m_a_substract_b_L && m_a_substract_b_R )
        m_a_substract_b_L = m_a_substract_b_R = false;

    m_b_substract_a_L = m_LeftB && !m_LeftA;
    m_b_substract_a_R = m_RightB && !m_RightA;
    //both in mean does not add to result.
    if ( m_b_substract_a_L && m_b_substract_a_R )
        m_b_substract_a_L = m_b_substract_a_R = false;

    m_intersect_L = m_LeftB && m_LeftA;
    m_intersect_R = m_RightB && m_RightA;
    //both in mean does not add to result.
    if ( m_intersect_L && m_intersect_R )
        m_intersect_L = m_intersect_R = false;

    m_exor_L = !( ( m_LeftB && m_LeftA ) || ( !m_LeftB && !m_LeftA ) );
    m_exor_R = !( ( m_RightB && m_RightA ) || ( !m_RightB && !m_RightA ) );
    //both in mean does not add to result.
    if ( m_exor_L && m_exor_R )
        m_exor_L = m_exor_R = false;
}


//put in direction with a_node as beginnode
void  kbLink::Redirect( kbNode* a_node )
{
    if ( a_node != m_beginnode )
    {
        // swap the begin- and endnode of the current link
        kbNode * dummy = m_beginnode;
        m_beginnode = m_endnode;
        m_endnode = dummy;

        bool swap = m_LeftA;
        m_LeftA = m_RightA;
        m_RightA = swap;

        swap = m_LeftB;
        m_LeftB = m_RightB;
        m_RightB = swap;

        swap = m_merge_L ;
        m_merge_L = m_merge_R;
        m_merge_R = swap;

        swap = m_a_substract_b_L;
        m_a_substract_b_L = m_a_substract_b_R;
        m_a_substract_b_R = swap;

        swap = m_b_substract_a_L;
        m_b_substract_a_L = m_b_substract_a_R;
        m_b_substract_a_R = swap;

        swap = m_intersect_L;
        m_intersect_L = m_intersect_R;
        m_intersect_R = swap;

        swap = m_exor_L;
        m_exor_L = m_exor_R;
        m_exor_R = swap;
    }
}

//
// This fucntion will ad a crossing to this line and the other line
// the crossing will be put in the link, because the line will be destructed
// after use of the variable
//
void kbLink::AddLineCrossing( B_INT X, B_INT Y, kbLink *other_line )
{
    // the other line must exist
    assert( other_line );
    // the links of the lines must exist
    assert( other_line );
    other_line->AddCrossing( AddCrossing( X, Y ) );
}

//
// Generate from the found crossings a part of the kbGraph
//
bool kbLink::ProcessCrossings( TDLI<kbLink>* _LI )
{
    kbNode * last; kbLink *dummy;
// assert (beginnode && endnode);

    if ( !linecrosslist ) return false;

    if ( linecrosslist->empty() ) return false;
    if ( linecrosslist->count() > 1 ) SortLineCrossings();
    GetEndNode()->RemoveLink( this );
    last = GetEndNode();
    // Make new links :
    while ( !linecrosslist->empty() )
    {
        dummy = new kbLink( GetGraphNum(), ( kbNode* ) linecrosslist->tailitem(), last, _GC );
        dummy->SetBeenHere();
        dummy->SetGroup( Group() );
        _LI->insbegin( dummy );
        last = ( kbNode* )linecrosslist->tailitem();
        linecrosslist->removetail();
    }
    // Recycle this link :
    last->AddLink( this );
    SetEndNode( last );
    delete linecrosslist;
    linecrosslist = NULL;
    return true;
}

static int NODE_X_ASCENDING_L ( kbNode* a, kbNode* b )
{
    if( b->GetX() > a->GetX() ) return( 1 );
    else
        if( b->GetX() == a->GetX() ) return( 0 );

    return( -1 );
}

static int NODE_X_DESCENDING_L( kbNode* a, kbNode* b )
{
    if( a->GetX() > b->GetX() ) return( 1 );
    else
        if( a->GetX() == b->GetX() ) return( 0 );

    return( -1 );
}

static int NODE_Y_ASCENDING_L ( kbNode* a, kbNode* b )
{
    if( b->GetY() > a->GetY() ) return( 1 );
    else
        if( b->GetY() == a->GetY() ) return( 0 );
    return( -1 );
}

static int NODE_Y_DESCENDING_L( kbNode* a, kbNode* b )
{
    if( a->GetY() > b->GetY() ) return( 1 );
    else
        if( a->GetY() == b->GetY() ) return( 0 );

    return( -1 );
}

//
// This function finds out which sortfunction to use with sorting
// the crossings.
//
void kbLink::SortLineCrossings()
{
    TDLI<kbNode> I( linecrosslist );

    B_INT dx, dy;
    dx = babs( GetEndNode()->GetX() - GetBeginNode()->GetX() );
    dy = babs( GetEndNode()->GetY() - GetBeginNode()->GetY() );
    if ( dx > dy )
    { // thislink is more horizontal then vertical
        if ( GetEndNode()->GetX() > GetBeginNode()->GetX() )
            I.mergesort( NODE_X_ASCENDING_L );
        else
            I.mergesort( NODE_X_DESCENDING_L );
    }
    else
    { // this link is more vertical then horizontal
        if ( GetEndNode()->GetY() > GetBeginNode()->GetY() )
            I.mergesort( NODE_Y_ASCENDING_L );
        else
            I.mergesort( NODE_Y_DESCENDING_L );
    }
}

//
// Adds a cross Node to this. a_node may not be deleted before processing the crossings
//
void kbLink::AddCrossing( kbNode *a_node )
{
    if ( a_node == GetBeginNode() || a_node == GetEndNode() ) return;


    if ( !linecrosslist )
    {
        linecrosslist = new DL_List<void*>();
        linecrosslist->insend( a_node );
    }
    else
    {
        TDLI<kbNode> I( linecrosslist );
        if ( !I.has( a_node ) )
            I.insend( a_node );
    }
}

//
// see above
//
kbNode* kbLink::AddCrossing( B_INT X, B_INT Y )
{
    kbNode * result = new kbNode( X, Y, _GC );
    AddCrossing( result );
    return result;
}

DL_List<void*>* kbLink::GetCrossList()
{
    if ( linecrosslist )
        return linecrosslist;
    return NULL;
}

bool kbLink::CrossListEmpty()
{
    if ( linecrosslist )
        return linecrosslist->empty();
    return true;
}

/*
bool kbLink::HasInCrossList(kbNode *n)
{
 if(linecrosslist!=NULL)
 {
  TDLI<kbNode> I(linecrosslist);
  return I.has(n);
 }
 return false;
}
*/

