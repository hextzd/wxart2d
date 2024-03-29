/*! \file src/graph.cpp
    \brief Used to Intercect and other process functions
    \author Klaas Holwerda 
 
    Copyright: 2001-2004 (C) Klaas Holwerda
 
    Licence: see kboollicense.txt 
 
    RCS-ID: $Id: graph.cpp,v 1.7 2009/09/14 16:50:12 titato Exp $
*/

// Grpah is the structure used to store polygons

#include "kbool/booleng.h"
#include "kbool/graph.h"
#include "kbool/graphlst.h"
#include "kbool/node.h"

// Prototype of function
int linkXYsorter( kbLink *, kbLink * );
int linkYXsorter( kbLink *, kbLink * );
int linkLsorter( kbLink *, kbLink * );
int linkYXtopsorter( kbLink *a, kbLink *b );
int linkGraphNumsorter( kbLink *_l1, kbLink* _l2 );

// constructor, initialize with one link
// usage: kbGraph *a_graph = new kbGraph(a_link);
kbGraph::kbGraph( kbLink *a_link, kbBool_Engine* GC )
{
    _GC = GC;
    _linklist = new DL_List<void*>();

    _linklist->insbegin( a_link );
    _bin = false;

}


// constructor, initialize graph with no contents
// usage: kbGraph *a_graph = new kbGraph();
kbGraph::kbGraph( kbBool_Engine* GC )
{
    _GC = GC;
    _linklist = new DL_List<void*>();
    _bin = false;
}

kbGraph::kbGraph( kbGraph* other )
{
    _GC = other->_GC;
    _linklist = new DL_List<void*>();
    _bin = false;

    int _nr_of_points = other->_linklist->count();
    kbLink* _current = other->GetFirstLink();

    kbNode* _last = _current->GetBeginNode();
    kbNode* node = new kbNode( _current->GetBeginNode()->GetX(), _current->GetBeginNode()->GetY(), _GC );
    kbNode* nodefirst = node;
    for ( int i = 0; i < _nr_of_points; i++ )
    {
        // get the other node on the link
        _last = _current->GetOther( _last );
        // get the other link on the _last node
        _current = _current->Forth( _last );

        kbNode* node2 = new kbNode( _current->GetBeginNode()->GetX(), _current->GetBeginNode()->GetY(), _GC );
        _linklist->insend( new kbLink( node,  node2, _GC ) );
        node = node2;
    }
    _linklist->insend( new kbLink( node,  nodefirst, _GC ) );
}

// destructor
// deletes all object of the linklist
kbGraph::~kbGraph()
{
    {
        TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );

        //first empty the graph
        _LI.delete_all();
    }

    delete _linklist;
}

kbLink* kbGraph::GetFirstLink()
{
    return ( kbLink* ) _linklist->headitem();
};


void kbGraph::Prepare( int intersectionruns )
{
    _GC->SetState( "Intersection" );

    bool found = true;
    int run = 0;
    while( run < intersectionruns && found )
    {
        found = CalculateCrossings( _GC->GetInternalMarge() );
        run++;
    }

//WHY
// Round(_GC->Get_Grid());

    {
        TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
        _LI.foreach_mf( &kbLink::UnMark );// Reset Bin and Mark flag
    }
    _GC->SetState( "Set group A/B Flags" );

    bool dummy = false;

    if ( _GC->GetWindingRule() )
        ScanGraph2( INOUT, dummy );

    ScanGraph2( GENLR, dummy );

    _GC->SetState( "Set operation Flags" );
    Set_Operation_Flags();

    writegraph( true );
    _GC->SetState( "Remove doubles" );

    // Remove the marked links
    {
        TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
        _LI.tohead();
        while( !_LI.hitroot() )
        {
            if ( _LI.item()->IsMarked() )
            {
                delete _LI.item();
                _LI.remove();
            }
            else
                _LI++;
        }
    }

    writegraph( true );
    _GC->SetState( "Remove inlinks" );
    Remove_IN_Links();

    writegraph( true );

    _GC->SetState( "Finished prepare graph" );
}



// x and y of the point will be rounded to the nearest
// xnew=N*grid and ynew=N*grid
void kbGraph::RoundInt( B_INT grid )
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.tohead();
    while ( !_LI.hitroot() )
    {
        _LI.item()->GetBeginNode()->RoundInt( grid );
        _LI.item()->GetEndNode()->RoundInt( grid );
        _LI++;
    }
}

// rotate graph minus 90 degrees or plus 90 degrees
void kbGraph::Rotate( bool plus90 )
{
    B_INT swap;
    kbNode* last = NULL;

    B_INT neg = -1;
    if ( plus90 )
        neg = 1;

    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.mergesort( linkXYsorter );

    _LI.tohead();
    while ( !_LI.hitroot() )
    {
        if ( _LI.item()->GetBeginNode() != last )
        {
            swap = _LI.item()->GetBeginNode()->GetX();
            _LI.item()->GetBeginNode()->SetX( -neg * ( _LI.item()->GetBeginNode()->GetY() ) );
            _LI.item()->GetBeginNode()->SetY( neg * swap );
            last = _LI.item()->GetBeginNode();
        }
        _LI++;
    }
}

bool kbGraph::RemoveNullLinks()
{
    bool graph_is_modified = false;

    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.tohead();
    while ( !_LI.hitroot() )
    {
        if ( _LI.item()->GetBeginNode() == _LI.item()->GetEndNode() )
        {
            _LI.item()->MergeNodes( _LI.item()->GetBeginNode() );
            delete _LI.item();
            _LI.remove();
            graph_is_modified = true;
        }
        else
            _LI++;
    }
    return ( graph_is_modified );
}

// Add a link to the graph connection with
// other links is through the link his nodes
void kbGraph::AddLink( kbLink *a_link )
{
    assert( a_link );

    _linklist->insend( a_link );
}

// Add a link to the graph, by giving it two nodes
// the link is then made and added to the graph
kbLink* kbGraph::AddLink( kbNode *begin, kbNode *end )
{
    assert( begin && end );
    assert( begin != end );
    kbLink* newlink = new kbLink( 0, begin, end, _GC );
    AddLink( newlink );
    return newlink;
}

// Checks if there is a zeroline in the graph
bool kbGraph::AreZeroLines( B_INT Marge )
{
    bool Found_it = false;

    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.tohead();
    while ( !_LI.hitroot() )
    {
        if ( _LI.item()->IsZero( Marge ) )
        {
            Found_it = true;
            break;
        }
        _LI++;
    }
    return Found_it;
}


// Delete links that do not fit the condition for given operation
void kbGraph::DeleteNonCond( BOOL_OP operation )
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.tohead();
    while( !_LI.hitroot() )
    {
        if ( !_LI.item()->IsMarked( operation ) )
        {
            delete _LI.item();
            _LI.remove();
        }
        else
            _LI++;
    }
}

void kbGraph::HandleNonCond( BOOL_OP operation )
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.tohead();
    while( !_LI.hitroot() )
    {
        if ( !_LI.item()->IsMarked( operation ) )
        {
            _LI.item()->SetBeenHere();
            _LI.item()->SetGraphNum( -1 );
        }
        _LI++;
    }
}

// All lines in the graph wich have a length < _GC->Get_Marge() will be deleted
//
// input : a marge, standard on _GC->Get_Marge()
// return: true if lines in the graph are deleted
//       : false if no lines in the graph are deleted
bool kbGraph::DeleteZeroLines( B_INT Marge )
{
    // Is the graph modified ?
    bool Is_Modified = false;
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );

    int Processed = _LI.count();

    _LI.tohead();
    while ( Processed > 0 )
    {
        Processed--;
        if ( _LI.item()->IsZero( Marge ) )
        {
            // the current link is zero, so make from both nodes one node
            // and delete the current link from this graph
            _LI.item()->MergeNodes( _LI.item()->GetBeginNode() );
            // if an item is deleted the cursor of the list is set to the next
            // so no explicit forth is needed
            delete _LI.item();
            _LI.remove();
            // we have to set Processed, because if a zero line is deleted
            // another can be made zero by this deletion
            Processed = _LI.count();
            Is_Modified = true;
            if ( _LI.hitroot() )
                _LI.tohead();
        }
        else
            _LI++;
        if ( _LI.hitroot() )
            _LI.tohead();
    }
    return Is_Modified;
}


// Collects a graph starting at currentnode or attached link
// follow graphs right around.
// since the input node is always a topleft node, we can see on
// a connected link if we or dealing with a hole or NON hole.
// for instance a top link of a hole that is horizontal, always
// is IN above the link and OUT underneath the link.
// this for a non hole the opposite
bool kbGraph::CollectGraph( kbNode *current_node, BOOL_OP operation, bool detecthole, int graphnumber )
{
    kbLink * currentlink;
    kbLink *nextlink;
    kbNode *next_node;
    kbNode *MyFirst;
    kbNode *Unlinked;
    kbLink *MyFirstlink;

    bool Hole;
    LinkStatus whatside;

    currentlink = current_node->GetNotFlat();
    if ( !currentlink )
    {
        char buf[100];
        if ( detecthole )
            sprintf( buf, "no NON flat link Collectgraph for operation at %15.3lf , %15.3lf",
                     double( current_node->GetX() ), double( current_node->GetY() ) );
        else
            sprintf( buf, "no NON flat link Collectgraph at %15.3lf , %15.3lf",
                     double( current_node->GetX() ), double( current_node->GetY() ) );
        throw kbBool_Engine_Error( buf, "Error", 9, 0 );
    }

    currentlink->SetBeenHere();

    if ( detecthole )
        Hole = currentlink->IsHole( operation );
    else
        Hole = currentlink->GetHole(); //simple extract do not detect holes, but use hole flag.

    currentlink->Redirect( current_node );

    //depending if we have a hole or not
    //we take the left node or right node from the selected link (currentlink)
    //this MEANS for holes go left around and for non holes go right around
    //since the currentlink is already set to binhere, it will not go in that direction
    if ( Hole )
    {
        whatside = IS_LEFT;
        if ( currentlink->GetEndNode()->GetX() > current_node->GetX() )
            current_node = currentlink->GetEndNode();
    }
    else
    {
        whatside = IS_RIGHT;
        if ( currentlink->GetEndNode()->GetX() < current_node->GetX() )
            current_node = currentlink->GetEndNode();
    }
    currentlink->Redirect( current_node );
    MyFirst = current_node; //remember this as the start
    MyFirstlink = currentlink;

    next_node = currentlink->GetEndNode();

    // If this is a hole, Set as special link, this is the top link of this hole !
    // from this link we have to make links to the link above later on.
    if ( Hole )
        currentlink->SetTopHole( true );
    //set also the link as being part of a hole
    if ( detecthole )
        currentlink->SetHole( Hole );
    currentlink->SetGraphNum( graphnumber );

    // Walk over links and redirect them. taking most right links around
    while ( currentlink != NULL )
    {
        if ( Hole )
        {
            nextlink = next_node->GetMost( currentlink, IS_RIGHT, operation );
        }
        else
        {
            nextlink = next_node->GetMost( currentlink, IS_LEFT, operation );
            // next works too if same is used in CollectGraphLast
            //nextlink = next_node->GetMost(currentlink, IS_RIGHT, operation);
        }

        if ( nextlink == NULL )
        { //END POINT MUST BE EQAUL TO BEGIN POINT
            if ( !next_node->Equal( MyFirst, 1 ) )
            {
                throw kbBool_Engine_Error( "no next (endpoint != beginpoint)", "graph", 9, 0 );

                //for god sake try this
                //nextlink = next_node->GetMost(currentlink, whatside ,operation);
            }
        }

        current_node = next_node;

        if ( nextlink != NULL )
        {
            nextlink->Redirect( current_node );
            nextlink->SetBeenHere();
            next_node = nextlink->GetEndNode();

            if ( current_node->GetNumberOfLinks() > 2 )
            {  // replace endnode of currentlink and beginnode of nextlink with new node
                Unlinked = new kbNode( current_node, _GC );
                currentlink->Replace( current_node, Unlinked );
                nextlink->Replace( current_node, Unlinked );
            }

            if ( detecthole )
                nextlink->SetHole( Hole );
            nextlink->SetGraphNum( graphnumber );
        }
        else
        {
            //close the found graph properly
            if ( current_node->GetNumberOfLinks() > 2 )
            {  // replace endnode of currentlink and beginnode of nextlink with new node
                Unlinked = new kbNode( current_node, _GC );
                currentlink->Replace( current_node, Unlinked );
                MyFirstlink->Replace( current_node, Unlinked );
            }
        }

        currentlink = nextlink;
    }

    //END POINT MUST BE EQAUL TO BEGIN POINT
    if ( !current_node->Equal( MyFirst, 1 ) )
    {
        throw kbBool_Engine_Error( "in collect graph endpoint != beginpoint", "Error", 9, 0 );
    }
    return Hole;
}

bool kbGraph::CollectGraphLast( kbNode *current_node, BOOL_OP operation, bool detecthole, int graphnumber )
{
    kbLink * currentlink;
    kbLink *nextlink;
    kbNode *next_node;
    kbNode *MyFirst;
    kbNode *Unlinked;
    kbLink *MyFirstlink;

    bool Hole;
    LinkStatus whatside;

    currentlink = current_node->GetNotFlat();
    if ( !currentlink )
    {
        char buf[100];
        if ( detecthole )
            sprintf( buf, "no NON flat link Collectgraph for operation at %15.3lf , %15.3lf",
                     double( current_node->GetX() ), double( current_node->GetY() ) );
        else
            sprintf( buf, "no NON flat link Collectgraph at %15.3lf , %15.3lf",
                     double( current_node->GetX() ), double( current_node->GetY() ) );
        throw kbBool_Engine_Error( buf, "Error", 9, 0 );
    }

    currentlink->SetBeenHere();

    if ( detecthole )
        Hole = currentlink->IsHole( operation );
    else
        Hole = currentlink->GetHole(); //simple extract do not detect holes, but use hole flag.

    currentlink->Redirect( current_node );

    //depending if we have a hole or not
    //we take the left node or right node from the selected link (currentlink)
    //this MEANS for holes go left around and for non holes go right around
    //since the currentlink is already set to binhere, it will not go in that direction
    if ( Hole )
    {
        whatside = IS_LEFT;
        if ( currentlink->GetEndNode()->GetX() > current_node->GetX() )
            current_node = currentlink->GetEndNode();
    }
    else
    {
        whatside = IS_RIGHT;
        if ( currentlink->GetEndNode()->GetX() < current_node->GetX() )
            current_node = currentlink->GetEndNode();
    }
    currentlink->Redirect( current_node );
    MyFirst = current_node; //remember this as the start
    MyFirstlink = currentlink;

    next_node = currentlink->GetEndNode();

    // If this is a hole, Set as special link, this is the top link of this hole !
    // from this link we have to make links to the link above later on.
    if ( Hole )
        currentlink->SetTopHole( true );
    currentlink->SetGraphNum( graphnumber );

    // Walk over links and redirect them. taking most right links around
    while ( currentlink != NULL )
    {
        if ( Hole )
        {
            if ( currentlink->GetHoleLink() )
            {
                //in case we entered the hole via the hole link just now, we follow the hole.
                //This is taking as many holes as possible ( most right around)
                nextlink = next_node->GetMostHole( currentlink, IS_RIGHT , operation, false );
                if ( !nextlink ) // hole done?
                    //if we did get to this hole via a holelink?, then we might now be on the return link.
                    //BTW it is also possible that holes are already found via a non linked hole path,
                    //in that case, we did go to the HoleLink here, and directly return on the other holelink.
                    nextlink = next_node->GetHoleLink( currentlink, IS_RIGHT, true, operation );
                if ( !nextlink )
                {
                    //we did get to this hole via a holelink and we are on the returning holelink.
                    //So we left the hole collection, and continue with contours.
                    //Most Right is needed!
                    nextlink = next_node->GetMost( currentlink, IS_RIGHT, operation );
                }
            }
            else
            {
                nextlink = next_node->GetMostHole( currentlink, IS_RIGHT, operation ); // other holes first
                if ( !nextlink )
                    nextlink = next_node->GetHoleLink( currentlink,  IS_RIGHT, true, operation ); // other linked holes first
                if ( !nextlink )
                {
                    //We are leaving the hole.
                    //So we left the hole collection, and continue with contours.
                    //Most Right is needed!
                    nextlink = next_node->GetMost( currentlink, IS_RIGHT, operation );
                }
            }
        }
        else
        {
            //nextlink = next_node->GetMost( currentlink, IS_RIGHT, operation );
            //if ( !nextlink )

            //a hole link is preferred above a normal link. If not no holes would be linked in anyway.
            nextlink = next_node->GetHoleLink( currentlink, IS_RIGHT, true, operation );
            if ( !nextlink )
                //also if we can get to a hole directly within a contour, that is better (get as much as possible)
                nextlink = next_node->GetMostHole( currentlink, IS_RIGHT, operation );
            if ( !nextlink )
                //if non of the above, we are still on the contour and take as must as possible to the left.
                //Like that we take as many contour togethere as possible.

                nextlink = next_node->GetMost( currentlink, IS_LEFT, operation );
            // next works too if same is used in CollectGraphLast
            //nextlink = next_node->GetMost(currentlink, IS_RIGHT, operation);
        }

        if ( nextlink == NULL )
        { //END POINT MUST BE EQAUL TO BEGIN POINT
            if ( !next_node->Equal( MyFirst, 1 ) )
            {
                throw kbBool_Engine_Error( "no next (endpoint != beginpoint)", "graph", 9, 0 );

                //for god sake try this
                //nextlink = next_node->GetMost(currentlink, whatside, operation);
            }
        }
        else
        {
            // when holes are already known, use the hole information to
            // go left are right around.
            Hole = nextlink->GetHole() || nextlink->GetHoleLink();
        }
        current_node = next_node;

        if ( nextlink != NULL )
        {
            nextlink->Redirect( current_node );
            nextlink->SetBeenHere();
            next_node = nextlink->GetEndNode();

            if ( current_node->GetNumberOfLinks() > 2 )
            {  // replace endnode of currentlink and beginnode of nextlink with new node
                Unlinked = new kbNode( current_node, _GC );
                currentlink->Replace( current_node, Unlinked );
                nextlink->Replace( current_node, Unlinked );
            }

            nextlink->SetGraphNum( graphnumber );
        }
        else
        {
            //close the found graph properly
            if ( current_node->GetNumberOfLinks() > 2 )
            {  // replace endnode of currentlink and beginnode of nextlink with new node
                Unlinked = new kbNode( current_node, _GC );
                currentlink->Replace( current_node, Unlinked );
                MyFirstlink->Replace( current_node, Unlinked );
            }
        }

        currentlink = nextlink;
    }

    //END POINT MUST BE EQAUL TO BEGIN POINT
    if ( !current_node->Equal( MyFirst, 1 ) )
    {
        throw kbBool_Engine_Error( "in collect graph endpoint != beginpoint", "Error", 9, 0 );
    }
    return Hole;
}
//==============================================================================
//==============================================================================

// Extract bi-directional graphs from this graph
// Mark the graphs also as being a hole or not.
void kbGraph::Extract_Simples( BOOL_OP operation, bool detecthole, bool& foundholes )
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    if ( _LI.empty() ) return;
    kbNode *begin;
    int graphnumber = 1;

    _LI.mergesort( linkYXtopsorter );
    _LI.tohead();
    while ( true )
    {
        begin = GetMostTopLeft( &_LI ); // from all the most Top nodes,
        // take the most left one
        // to most or not to most, that is the question
        if ( !begin )
            break;

        try // collect the graph
        {
            if ( detecthole )
            {   
                bool hole = CollectGraph( begin, operation, detecthole, graphnumber );
                foundholes = hole || foundholes;
                graphnumber++;
            }
            else
            {
                //bool hole = CollectGraph( begin, operation, detecthole, graphnumber++ );
                bool hole = CollectGraphLast( begin, operation, detecthole, graphnumber );
                foundholes = hole || foundholes;
                graphnumber++;
            }
        }
        catch ( kbBool_Engine_Error & error )
        {
            _GC->info( error.GetErrorMessage(), "error" );
            throw error;
        }
    }
}

void kbGraph::Split( kbGraphList* partlist )
{
    int graphnumber = 0;
    std::list<int> todo;

    // graphnumber = 0 is reserved for non handled links.
    todo.push_back( graphnumber );

    {
        //sort the graph on graphnumber
        TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
        _LI.mergesort( linkGraphNumsorter );
    }

    kbGraph *part = NULL;
    while ( ! todo.empty() )
    {
        graphnumber =  todo.front();
        part = CollectGraphNumber( graphnumber, todo );
        if ( part )
        {
            if ( graphnumber == 0 )
            {
                delete part;
            }
            else
            {
                partlist->insend( part );
            }
        }
        todo.erase( todo.begin( ) );
        if ( todo.empty() )
        {
            if ( _linklist->count() )
                todo.push_back( ((kbLink*) _linklist->headitem())->GetGraphNum() );        
        }
    }
}

kbGraph* kbGraph::CollectGraphNumber( int graphnumber, kbGraphNumList& todo )
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    if ( _LI.empty() ) return NULL;

    kbGraph *part = NULL;

    _LI.tohead();
    while ( !_LI.hitroot() )
    {
        kbLink* link = _LI.item();
        if ( graphnumber == link->GetGraphNum() )
        {
            if ( !part )
                part = new kbGraph( _GC );

            if ( !link->CrossListEmpty() )
            {
                DL_List<void*>* crosslist = link->GetCrossList();
                while ( !crosslist->empty() )
                {
                    kbNode* topNode = ((kbNode*)crosslist->tailitem());
                    kbLink* linkHole = topNode->GetHoleFlaggedLink();
                    if ( linkHole )
                    {
                        int graphnumberLinked = linkHole->GetGraphNum();
                        todo.push_back( graphnumberLinked );
                    }
                    crosslist->removetail();
                }
            }
            part->AddLink( _LI.item() );
            _LI.remove();
        }
        else
            _LI++;
    }
    return part;
}

bool kbGraph::GetBeenHere()
{
    return _bin;
}

// return total number of links in this graph
int kbGraph::GetNumberOfLinks()
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    return _LI.count();
}

//for all operations set the operation flags for the links
//based on the Group_Left_Right values
void kbGraph::Set_Operation_Flags()
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.tohead();
    while( !_LI.hitroot() )
    {
        _LI.item()->SetLineTypes();
        _LI++;
    }
}

//  Remove unused (those not used for any operation) links
void kbGraph::Remove_IN_Links()
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.tohead();
    for ( int t = _LI.count() ; t > 0; t-- )
    {
        // Is this link not used for any operation?
        if ( _LI.item()->IsUnused() )
        {
            delete _LI.item();
            _LI.remove();
        }
        else
            _LI++;
    }
}

void kbGraph::ResetBinMark()
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    if ( _LI.empty() ) return;
    _LI.foreach_mf( &kbLink::UnMark );//reset bin and mark flag of each link
}

void kbGraph::ReverseAllLinks()
{
    kbNode * dummy;
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.tohead();
    while ( !_LI.hitroot() )
    {
        // swap the begin- and endnode of the current link
        dummy = _LI.item()->GetBeginNode();
        _LI.item()->SetBeginNode( _LI.item()->GetEndNode() );
        _LI.item()->SetEndNode( dummy );
        _LI++;
    }
}

void kbGraph::SetBeenHere( bool value )
{
    _bin = value;
}

// ReSet the flags  of the links
void kbGraph::Reset_flags()
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.foreach_mf( &kbLink::Reset_flags );
}

// ReSet the bin and mark flag of the links
void kbGraph::Reset_Mark_and_Bin()
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.foreach_mf( &kbLink::UnMark );//reset bin and mark flag of each link
}

// Set the group of the links to the same as newgroup
void kbGraph::SetGroup( GroupType newgroup )
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.tohead();
    while ( !_LI.hitroot() )
    {
        _LI.item()->SetGroup( newgroup );
        _LI++;
    }
}


// Set the number of the links to the same as newnr
void kbGraph::SetNumber( const int newnr )
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.tohead();
    while ( !_LI.hitroot() )
    {
        _LI.item()->SetGraphNum( newnr );
        _LI++;
    }
}


// This function will simplify a graph with the following rules
//
// This are the rules for symplifying the graphs
// 1. The following point is the same as the current one
// 2. The point after the following point is the same as the current one
// 3. The point after the following point lies on the same line as the current
//
// input : a marge
// return: true if graph is modified
//   : false if graph is NOT simplified
bool kbGraph::Simplify( B_INT Marge )
{
    bool graph_is_modified = false;
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    int Processed = _LI.count();

    _LI.foreach_mf( &kbLink::UnMark );//reset bin and mark flag of each link

    _LI.tohead();
    GroupType mygroup = _LI.item()->Group();

    // All items must be processed
    while ( Processed > 0 )
    {
        // Gives the number of items to process
        Processed--;
        // Check if line is marked
        // Links will be marked during the process
        if ( _LI.item()->IsMarked() )
        {
            delete _LI.item();
            _LI.remove();
            graph_is_modified = true;
            Processed = _LI.count();
            if ( _LI.hitroot() )
                _LI.tohead();
            continue;
        }

        // Line is not marked, check if line is zero
        if ( !_LI.item()->GetArcPiece() && _LI.item()->IsZero( Marge ) )
        {
            _LI.item()->MergeNodes( _LI.item()->GetBeginNode() );
            delete _LI.item();
            _LI.remove();
            graph_is_modified = true;
            Processed = _LI.count();
            if ( _LI.hitroot() )
                _LI.tohead();
            continue;
        }

        if ( _LI.item()->GetArcPiece() )
        {
            _LI++;
            if ( _LI.hitroot() )
                _LI.tohead();
                continue;
        }

        // begin with trying to simplify the link
        {
            // Line is not marked, not zero, so maybe it can be simplified
            bool virtual_link_is_modified;
            kbNode *new_begin, *new_end, *a_node;
            kbLink *a_link;

            _LI.item()->Mark();
            new_begin = _LI.item()->GetBeginNode();
            new_end   = _LI.item()->GetEndNode();

            // while virtual link is modified
            do
            {
                virtual_link_is_modified = false;
                // look in the previous direction
                if ( ( a_link = new_begin->GetPrevLink() ) != NULL )
                {
                    if ( !a_link->GetArcPiece() )
                    {
                        a_node = a_link->GetBeginNode();
                        if ( a_node->Simplify( new_begin, new_end, Marge ) )
                        {
                            new_begin->GetPrevLink()->Mark();
                            new_begin = a_node;
                            virtual_link_is_modified = true;
                        }
                    }
                }
                // look in the next direction
                if ( ( a_link = new_end->GetNextLink() ) != NULL )
                {
                    if ( !a_link->GetArcPiece() )
                    {
                        a_node = a_link->GetEndNode();
                        if ( a_node->Simplify( new_begin, new_end, Marge ) )
                        {
                            new_end->GetNextLink()->Mark();
                            new_end = a_node;
                            virtual_link_is_modified = true;
                        }
                    }
                }
                graph_is_modified = ( bool ) ( graph_is_modified || virtual_link_is_modified );
            }
            while ( virtual_link_is_modified );

            // was the link simplified
            if ( ( _LI.item()->GetBeginNode() != new_begin ) ||
                    ( _LI.item()->GetEndNode() != new_end ) )
            {
                // YES !!!!!
                int number = _LI.item()->GetGraphNum();
                delete _LI.item();
                _LI.remove();

                if ( _LI.hitroot() )
                    _LI.tohead();

                kbLink *newlink = new kbLink( number, new_begin, new_end, _GC );
                newlink->SetGroup( mygroup );

                _LI.insend( newlink );
                Processed = _LI.count();
                graph_is_modified = true;
                continue;
            }
            _LI.item()->UnMark();
        } // end of try to simplify
        _LI++;
        if ( _LI.hitroot() )
            _LI.tohead();
    }//end while all processed

    return graph_is_modified;
}

/*
// This function will smoothen a graph with the following rules
//
// 0. Process graphs with more than 3 links only. (while more than 3)
//  Otherwise some objects may end-up as lines or disappear completely.
// 1.
//  a. ?  Does begin-node lay on line(prevline.beginnode,endnode)
//     ->  merge beginnode to endnode
//  b. ?  Does end-node lay on line(beginnode,nextline.endnode)
//     ->  merge endnode to beginnode
// 2.
//  a. ?  Is distance between prevline.beginnode and endnode to short
//     ->  merge beginnode to endnode
//   b. ?  Is distance between beginnode and nextline.endnode to short
//     ->  merge endnode to beginnode
// 3.
//  a. ?  Does (this)beginnode lay in area of nextline
//    AND does cross-node lay on nextline
//   ->   move endnode to crossing of prevline and nextline
//  b. ?  Does (this)endnode lay in area of prevline
//    AND does cross-node lay on prevline
//   ->   move beginnode to crossing of prevline and nextline
// 4.
//  ?  Is this link too short
//   ?  Is prevline shorter than nextline
//     Y ->  ?  Is prevlink shorter than maxlength
//     ->  merge endnode to beginnode
//     N ->  ?  Is nextlink shorter than maxlength
//     ->  merge endnode to beginnode
//
//
// Types of glitches / lines to remove :
//
//    \         /      \     /       \         /
//   Z---A---B  OR  Z-------B---A    =>   Z-------B
//
//   (1)
//
//   ----A    C----          =>  ----A-----C----
//    \   /
//   (2)   \ /
//        B
//
//   ---Z                                          ---Z
//       \                                                \
//   (3)  \                                                \
//         \   B----------C--       =>          A---B----------C--
//          \ /
//           A
//
//   --Z---A                                          --Z__
//          \                                              -__
//   (4)     B------------C--       =>            B------------C--
//
// linkLsorter(L1,L2)
//  ret:
//   +1  L1 < L2
//    0  L1 == L2
//   -1  L1 > L2
//
*/
bool kbGraph::Smoothen( B_INT Marge )
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    if ( _LI.count() <= 3 ) // Don't modify it
        return false;

    kbNode *Z, *A, *B, *C, *cross_node = new kbNode( _GC );
    kbLink *prevlink, *nextlink, *thislink;
    kbLine prevline( _GC ),  nextline( _GC ),  thisline( _GC );
    kbLine prevhelp( _GC ),  nexthelp( _GC );

    kbLink  LZB( new kbNode( _GC ), new kbNode( _GC ), _GC ),
    LAC( new kbNode( _GC ), new kbNode( _GC ), _GC );

    double distance = 0;
    double prevdist, nextdist;

    bool doprev, donext;
    bool graph_is_modified = false;
    bool kill = false; // for first instance

    _LI.tohead();
    int todo = _LI.count();
    thislink = _LI.item();
    B = thislink->GetEndNode();
    nextlink = thislink->Forth( B );

    // Type 1
    while ( todo > 0 )
    {
        if ( kill == true )
        {
            // remove link from graph
            _LI.toitem( thislink );
            graph_is_modified = true;
            delete _LI.item();
            _LI.remove();
            kill = false;
            thislink = nextlink;
            todo--;
            if ( _LI.count() < 3 )
                break;
        }

        A = thislink->GetBeginNode();
        B = thislink->GetEndNode();

        if ( A->ShorterThan( B, 1 ) )
        {
            A->Merge( B );
            kill = true;
            continue;
        }

        Z = thislink->Forth( A )->GetBeginNode();
        C = thislink->Forth( B )->GetEndNode();
        thisline.Set( thislink );
        thisline.CalculateLineParameters();
        nextlink = thislink->Forth( B );

        if ( thisline.PointInLine( Z, distance, 0.0 ) == ON_AREA )
        { // Z--A--B =>  Z--B       Merge this to previous
            thislink->MergeNodes( B ); // remove A
            kill = true;
            continue;
        }
        else if ( thisline.PointInLine( C, distance, 0.0 ) == ON_AREA )
        { // A--B--C =>  A--C       Merge this to next
            thislink->MergeNodes( A ); // remove B
            kill = true;
            continue;
        }

        thislink = nextlink;
        todo--;
    }

    kill = false;
    todo = _LI.count();
    _LI.mergesort( linkLsorter );
    _LI.tohead();

    while ( todo > 0 )
    {
        if ( kill == true )
        {
            delete _LI.item();
            _LI.remove();
            graph_is_modified = true;
            kill = false;
            //mergesort(linkLsorter);
            _LI.mergesort( linkLsorter );
            _LI.tohead();
            todo = _LI.count();
            if ( todo < 3 )  // A polygon, at least, has 3 sides
                break;
        }

        // Keep this order!
        thislink = _LI.item();
        A = thislink->GetBeginNode();
        B = thislink->GetEndNode();
        prevlink = thislink->Forth( A );
        nextlink = thislink->Forth( B );
        Z = prevlink->GetBeginNode();
        C = nextlink->GetEndNode();

        if ( A->ShorterThan( B, 1 ) )
        {
            A->Merge( B );
            kill = true;
            continue;
        }

        prevline.Set( prevlink );
        prevline.CalculateLineParameters();
        nextline.Set( nextlink );
        nextline.CalculateLineParameters();

        // Type 2
        if ( B->ShorterThan( Z, Marge ) )
        { // Z--A--B =>  Z--B       Merge this to previous
            thislink->MergeNodes( B ); // remove A
            kill = true;
            continue;
        }
        else if ( A->ShorterThan( C, Marge ) )
        { // A--B--C =>  A--C       Merge this to next
            thislink->MergeNodes( A ); // remove B
            kill = true;
            continue;
        }


        *LZB.GetBeginNode() = *Z;
        *LZB.GetEndNode() = *B;
        *LAC.GetBeginNode() = *A;
        *LAC.GetEndNode() = *C;
        prevhelp.Set( &LZB );
        nexthelp.Set( &LAC );
        prevhelp.CalculateLineParameters();
        nexthelp.CalculateLineParameters();


        // Type 4
        doprev = bool( prevhelp.PointInLine( A, prevdist, ( double )Marge ) == IN_AREA );
        donext = bool( nexthelp.PointInLine( B, nextdist, ( double )Marge ) == IN_AREA );
        doprev = bool( B->ShorterThan( Z, _GC->GetInternalMaxlinemerge() ) && doprev );
        donext = bool( A->ShorterThan( C, _GC->GetInternalMaxlinemerge() ) && donext );

        if ( doprev && donext )
        {
            if ( fabs( prevdist ) <= fabs( nextdist ) )
                thislink->MergeNodes( B ); // remove A
            else
                thislink->MergeNodes( A ); // remove B

            kill = true;
            continue;
        }
        else if ( doprev )
        {
            thislink->MergeNodes( B ); // remove A
            kill = true;
            continue;
        }
        else if ( donext )
        {
            thislink->MergeNodes( A ); // remove B
            kill = true;
            continue;
        }


        thisline.Set( thislink );
        thisline.CalculateLineParameters();

        // Type 1
        if ( thisline.PointInLine( Z, distance, 0.0 ) == ON_AREA )
        { // Z--A--B =>  Z--B       Merge this to previous
            thislink->MergeNodes( B ); // remove A
            kill = true;
            continue;
        }
        else if ( thisline.PointInLine( C, distance, 0.0 ) == ON_AREA )
        { // A--B--C =>  A--C       Merge this to next
            thislink->MergeNodes( A ); // remove B
            kill = true;
            continue;
        }


        // Type 3
        if ( nextline.PointInLine( A, distance, ( double ) Marge ) == IN_AREA )
        {
            if ( nextline.Intersect2( cross_node, &prevline ) )
            {
                if ( nextline.PointInLine( cross_node, distance, 0.0 ) == IN_AREA )
                {
                    B->Set( cross_node );
                    thislink->MergeNodes( B ); // remove A
                    kill = true;
                    continue;
                }
                else
                {
                    thislink->MergeNodes( A ); // remove B
                    kill = true;
                    continue;
                }
            }
            else
            {
                thislink->MergeNodes( A ); // remove B
                kill = true;
                continue;
            }
        }

        // Type 3
        if ( prevline.PointInLine( B, distance, ( double )Marge ) == IN_AREA )
        {
            if ( prevline.Intersect2( cross_node, &nextline ) )
            {
                if ( prevline.PointInLine( cross_node, distance, 0.0 ) == IN_AREA )
                {
                    A->Set( cross_node );
                    thislink->MergeNodes( A ); // remove B
                    kill = true;
                    continue;
                }
                else
                {
                    thislink->MergeNodes( B ); // remove A
                    kill = true;
                    continue;
                }
            }
            else
            {
                thislink->MergeNodes( B ); // remove A
                kill = true;
                continue;
            }
        }

        todo--;
        _LI++;
    } // end: while all processed

    delete cross_node;

    return graph_is_modified;
}


// Give the graphnumber of the first link in the graphlist
int kbGraph::GetGraphNum()
{
    return ( ( kbLink* )_linklist->headitem() )->GetGraphNum();
}


// get the node with the highest Y value
kbNode* kbGraph::GetTopNode()
{
    B_INT max_Y = MAXB_INT;
    kbNode* result;

    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );

    _LI.tohead();
    while ( !_LI.hitroot() )
    {
        if ( !( _LI.item()->GetBeginNode()->GetY() < max_Y ) )
            break;
        _LI++;
    }
    result = _LI.item()->GetBeginNode();

    return result;
}

// THE GRAPH MUST be SORTED before using this function
// mergesort(linkYXtopsorter);
// Get the mostleft top node from the graph  for which the link flag is not set yet
kbNode* kbGraph::GetMostTopLeft( TDLI<kbLink>* _LI )
{
    while ( !_LI->hitroot() )
    {
        if ( !_LI->item()->BeenHere() )
        {
            kbLink * a = _LI->item();
            //find the top node of this link (sorted on top allready)
            if ( a->GetBeginNode()->GetY() > a->GetEndNode()->GetY() )
                return( a->GetBeginNode() );
            if ( a->GetBeginNode()->GetY() < a->GetEndNode()->GetY() )
                return( a->GetEndNode() );
            else
                return( a->GetBeginNode() );
        }
        ( *_LI )++;
    }
    return NULL;
}

// Take the linkslist over from a other graph
// The linklist of the other graph will be empty afterwards
void kbGraph::TakeOver( kbGraph *other )
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.takeover( other->_linklist );
}

// calculate crossing with scanbeams
bool kbGraph::CalculateCrossings( B_INT Marge )
{
    // POINT <==> POINT
    _GC->SetState( "Node to Node" );

    bool found = false;
    bool dummy = false;

    writegraph(true);
    found = Merge_NodeToNode( Marge ) != 0;
    writegraph(true);

    if ( _linklist->count() < 3 )
        return found;

    // POINT <==> LINK
    _GC->SetState( "Node to kbLink 0" );

    found = ScanGraph2( NODELINK, dummy ) != 0 || found;
    writegraph(true);

    _GC->SetState( "Rotate -90" );
    Rotate( false );

    _GC->SetState( "Node to kbLink -90" );
    found = ScanGraph2( NODELINK, dummy ) != 0 || found;
    writegraph(true);

    _GC->SetState( "Rotate +90" );
    Rotate( true );

    // LINK <==> LINK
    _GC->SetState( "intersect" );

    found = ScanGraph2( LINKLINK, dummy ) != 0 || found;

    /*
       if (!checksort())
       { {
        TDLI<kbLink> _LI=TDLI<kbLink>(_linklist);
      _LI.mergesort(linkXYsorter);
         }
       writeintersections();
       writegraph(true);
       }
    */

// Rotate(false);
// _GC->SetState("kbLink to kbLink -90");
//   ScanGraph2(LINKLINK);
// Rotate(true);

    writegraph( true );

    _GC->Write_Log( "Node to Node" );
    _GC->SetState( "Node to Node" );

    found = Merge_NodeToNode( Marge ) != 0 || found;
    writegraph( true );

    return found;
}

// neem de nodes die binnen de margeafstand bij elkaar liggen samen RICHARD
int kbGraph::Merge_NodeToNode( B_INT Marge )
{
    //aantal punten dat verplaatst is
    int merges = 0;
    {
        TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );

        //unmark all links; markflag wordt gebruikt om aan te geven
        //of een link (eigenlijk beginnode ervan) al verwerkt is
        _LI.foreach_mf( &kbLink::UnMark );

        //sort links on x value of beginnode
        _LI.mergesort( linkXYsorter );

        //extra iterator voor doorlopen links in graph
        {
            TDLI<kbLink>  links = TDLI<kbLink>( _linklist );

            kbNode *nodeOne, *nodeTwo;
            //verwerk alle links (alle (begin)nodes)
            for( _LI.tohead(); !_LI.hitroot(); _LI++ )
            {
                nodeOne = _LI.item()->GetBeginNode();

                // link (beginnode) al verwerkt?
                if( !_LI.item()->IsMarked() )
                {
                    // wordt verwerkt dus markeer
                    _LI.item()->Mark();

                    // doorloop alle links vanaf huidige tot link buiten marge
                    links.toiter( &_LI );links++;
                    while ( !links.hitroot() )
                    {
                        nodeTwo = links.item()->GetBeginNode();

                        // marked?
                        if( !links.item()->IsMarked() )
                        {
                            // x van beginnode vd link binnen marge?
                            if( babs( nodeOne->GetX() - nodeTwo->GetX() ) <= Marge )
                            {
                                // y van beginnode vd link binnen marge?
                                // zijn twee node-object referenties wel verschillend?
                                if( babs( nodeOne->GetY() - nodeTwo->GetY() ) <= Marge &&
                                        ( !( nodeOne == nodeTwo ) )
                                  )
                                {
                                    links.item()->Mark();
                                    nodeOne->Merge( nodeTwo );
                                    merges++;
                                }//y binnen marge en niet zelfde node
                            }//x binnen marge?
                            else
                            {
                                // link valt buiten marge; de rest vd links
                                // dus ook (omdat lijst gesorteerd is)
                                links.totail();
                            }
                        }//marked?
                        links++;
                    }//current -> ongeldig
                }//verwerkt?
            }//all links

        }//om de extra iterator te verwijderen
    }
    RemoveNullLinks();

    return merges;
}


int kbGraph::ScanGraph2( SCANTYPE scantype, bool& holes )
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    int found = 0;
    int count = 0;
    char buf[100];

    writegraph( false );
    //sort links on x and y value of beginnode
    _LI.mergesort( linkXYsorter );

    writegraph( true );

    //bin flag is used in scanbeam so reset
    _LI.foreach_mf( &kbLink::SetNotBeenHere );

    ScanBeam* scanbeam = new ScanBeam( _GC );
    kbNode*  _low;
    kbNode*  _high;

    _LI.tohead();
    while ( !_LI.attail() )
    {
        _low = _LI.item()->GetBeginNode();
        //find new links for the new beam and remove the old links
        if ( scanbeam->FindNew( scantype, &_LI, holes ) )
            found++;

        //find a new low node, this should be a node not eqaul to the current _low
        do
        {  _LI++;}
        while ( !_LI.hitroot() && ( _low == _LI.item()->GetBeginNode() ) );
        if ( _LI.hitroot() )
            //if the last few links share the same beginnode
            //we arive here
            break;
        else
            _high = _LI.item()->GetBeginNode();

        scanbeam->SetType( _low, _high );

        if ( scanbeam->RemoveOld( scantype, &_LI, holes ) )
            found++;

        count++;
        sprintf( buf, "graph count %d", count );
        _GC->SetState( buf );        
        writegraph( true );

    }

    delete scanbeam;
    return found;
}


/*
 
//       scanbeam->writebeam();
 
      if (j%100 ==0)
      {
        long x;
        long y;
        char buf[80];
     x=(long)_lowlink->GetBeginNode()->GetX();
     y=(long)_lowlink->GetBeginNode()->GetY();
        sprintf(buf," x=%I64d , y=%I64d here %I64d",x,y,scanbeam->count());
   _GC->SetState(buf);
       scanbeam->writebeam();
      }
 
 
 
         writegraph(false);
            if (!checksort())
            {
               double x=_lowlink->GetBeginNode()->GetX();
               checksort();
            }
 
 
 
         _LI++;
      }
   }
 
 delete scanbeam;
 return 0;
}
 
 
         if (!checksort())
         {
            x=_lowlink->GetBeginNode()->GetX();
            checksort();
         }
 
         if (x >= -112200)
         {
//          writegraph(true);
//          scanbeam->writebeam();
         }
*/


//=============================== Global Functions =============================

// Sorts the links on the X values
int linkXYsorter( kbLink *a, kbLink* b )
{
    if ( a->GetBeginNode()->GetX() < b->GetBeginNode()->GetX() )
        return( 1 );
    if ( a->GetBeginNode()->GetX() > b->GetBeginNode()->GetX() )
        return( -1 );
    //they are eqaul in x
    if ( a->GetBeginNode()->GetY() < b->GetBeginNode()->GetY() )
        return( -1 );
    if ( a->GetBeginNode()->GetY() > b->GetBeginNode()->GetY() )
        return( 1 );
    //they are eqaul in y
    return( 0 );
}

// Sorts the links on the Y value of the beginnode
int linkYXsorter( kbLink *a, kbLink* b )
{
    if ( a->GetBeginNode()->GetY() > b->GetBeginNode()->GetY() )
        return( 1 );
    if ( a->GetBeginNode()->GetY() < b->GetBeginNode()->GetY() )
        return( -1 );
    if ( a->GetBeginNode()->GetX() > b->GetBeginNode()->GetX() )
        return( -1 );
    if ( a->GetBeginNode()->GetX() < b->GetBeginNode()->GetX() )
        return( 1 );
    return( 0 );
}


// The sort function for sorting graph from shortest to longest (_l1 < _l2)
int linkLsorter( kbLink *_l1, kbLink* _l2 )
{
    B_INT dx1, dx2, dy1, dy2;
    dx1 = ( _l1->GetEndNode()->GetX() - _l1->GetBeginNode()->GetX() );
    dx1 *= dx1;
    dy1 = ( _l1->GetEndNode()->GetY() - _l1->GetBeginNode()->GetY() );
    dy1 *= dy1;
    dx2 = ( _l2->GetEndNode()->GetX() - _l2->GetBeginNode()->GetX() );
    dx2 *= dx2;
    dy2 = ( _l2->GetEndNode()->GetY() - _l2->GetBeginNode()->GetY() );
    dy2 *= dy2;

    dx1 += dy1; dx2 += dy2;

    if ( dx1 > dx2 )
        return( -1 );
    if ( dx1 < dx2 )
        return( 1 );
    return( 0 );
}

// The sort function for the links in a graph (a.topnode < b.topnode)
// if the two links lay with the top nodes on eachother the most left will be selected

int linkYXtopsorter( kbLink *a, kbLink *b )
{

    if ( bmax( a->GetBeginNode()->GetY(), a->GetEndNode()->GetY() ) < bmax( b->GetBeginNode()->GetY(), b->GetEndNode()->GetY() ) )
        return -1;

    if ( bmax( a->GetBeginNode()->GetY(), a->GetEndNode()->GetY() ) > bmax( b->GetBeginNode()->GetY(), b->GetEndNode()->GetY() ) )
        return 1;

    //equal
    if ( bmin( a->GetBeginNode()->GetX(), a->GetEndNode()->GetX() ) < bmin( b->GetBeginNode()->GetX(), b->GetEndNode()->GetX() ) )
        return -1;

    if ( bmin( a->GetBeginNode()->GetX(), a->GetEndNode()->GetX() ) > bmin( b->GetBeginNode()->GetX(), b->GetEndNode()->GetX() ) )
        return 1;

    return 0;
}

// The sort function for sorting graph from shortest to longest (_l1 < _l2)
int linkGraphNumsorter( kbLink *_l1, kbLink* _l2 )
{
    if ( _l1->GetGraphNum() > _l2->GetGraphNum() )
        return( -1 );
    if ( _l1->GetGraphNum() < _l2->GetGraphNum() )
        return( 1 );
    return( 0 );
}

// Perform an operation on the graph
void kbGraph::Boolean( BOOL_OP operation, kbGraphList* Result )
{
    _GC->SetState( "Performing Operation" );

    // At this moment we have one graph
    // step one, split it up in single graphs, and mark the holes
    // step two, make one graph again and link the holes
    // step three, split up again and dump the result in Result

    _GC->SetState( "Extract simples first " );

    ResetBinMark();
    DeleteNonCond( operation );
    HandleNonCond( operation );

    bool foundholes = false;
    try
    {
        if ( _GC->GetDebug() )
           WriteGraphKEY( _GC, "inputToKbool.key" );
        writegraph( true );

        Extract_Simples( operation, true, foundholes );
    }
    catch ( kbBool_Engine_Error & error )
    {
        throw error;
    }

    // now we will link all the holes in de graphlist
    // By the scanbeam method we will search all the links that are marked
    // as topleft link of a the hole polygon, when we find them we will get the
    // closest link, being the one higher in the beam.
    // Next we will create a link and nodes toceoonect the hole into it outside contour
    // or other hole.
    _GC->SetState( "Linking Holes" );

    if ( _linklist->count() == 0 )
        //extract simples did leaf an empty graph
        //so we are ready
        return;

    if ( foundholes )
    {
        //the first extract simples introduced nodes at the same location that are not merged.
        //e.g. Butterfly polygons as two seperate polygons.
        //The scanlines can not cope with this, so merge them, and later extract one more time.
        Merge_NodeToNode( 0 );

        //_GC->SetLog( true ); 
        _GC->Write_Log( "LINKHOLES\n" );
        writegraph( false );

        //link the holes into the non holes if there are any.
        bool holes = false;
        ScanGraph2( LINKHOLES, holes );

        if ( _GC->GetDebug() )
            WriteGraphKEY( _GC, "afterHoleLinkage.key" );
        writegraph( true );
        if ( holes )
        {
            //to delete extra points
            //those extra points are caused by link holes
            //and are eqaul ( the smallest number in integer is 1 )
            DeleteZeroLines( 1 );

            _GC->SetState( "extract simples last" );
            ResetBinMark();
            HandleNonCond( operation );
            DeleteNonCond( operation );
            writegraph( true );
            Extract_Simples( operation, false, foundholes );
        }
    }

    //writegraph( false );
    Split( Result );
}

// Perform an correction on the graph
void kbGraph::Correction( kbGraphList* Result, double factor )
{
    // At this moment we have one graph
    // step one, split it up in single graphs, and mark the holes
    // step two, make one graph again and link the holes
    // step three, split up again and dump the result in Result
    _GC->SetState( "Extract simple graphs" );

    //extract the (MERGE or OR) result from the graph
    if ( Simplify( _GC->GetGrid() ) )
        if ( GetNumberOfLinks() < 3 )
            return;

    //copy this into original ( holes also extracted 
    kbGraph* original = new kbGraph( _GC );
    {
        if ( _linklist->empty() ) return;

/* this version forgets holes which are not linked, only contour is extracted.
        kbLink* _current = GetFirstLink();
        kbNode *_first = new kbNode( _current->GetBeginNode(), _GC );
        kbNode *_last  = _current->GetBeginNode();
        kbNode *_begin = _first;
        kbNode *_end   = _first;

        int _nr_of_points = GetNumberOfLinks();
        for ( int i = 1; i < _nr_of_points; i++ )
        {
            // get the other node on the link
            _last = _current->GetOther( _last );
            // make a node from this point
            _end = new kbNode( _last, _GC );

            // make a link between begin and end
            original->AddLink( _begin, _end );
            if ( _current->GetHole() ) 
                _current->SetHole( true );

            _begin = _end;
            _current = _current->Forth( _last );
        }
*/
        // bin flag used to only extract a contour or hole once.
        ResetBinMark();
        TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
        _LI.tohead();
        while ( !_LI.hitroot() )
        {
            kbLink* _current = _LI.item();
            if ( !_current->BeenHere() )
            {
                kbNode *_first = new kbNode( _current->GetBeginNode(), _GC );
                kbNode *_last  = _current->GetBeginNode();
                kbNode *_start = _current->GetBeginNode();
                kbNode *_begin = _first;
                kbNode *_end   = _first;

                do {
                    _current->SetBeenHere();
                    // get the other node on the link
                    _last = _current->GetOther( _last );
                    // make a node from this point
                    _end = new kbNode( _last, _GC );

                    // make a link between begin and end
                    original->AddLink( _begin, _end );
                    if ( _current->GetHole() ) 
                        _current->SetHole( true );

                    _begin = _end;
                    _current = _current->Forth( _last );
                }
                while ( _start != _last );
                // make a link between the _begin and the first to close the graph
                original->AddLink( _begin, _first );
            }
            _LI++;
        }
    }

    SetNumber( 1 );
    SetGroup( GROUP_A );
    Prepare( 1 );
    ResetBinMark();
    //DeleteNonCond(BOOL_OR);
    HandleNonCond( BOOL_OR );

    bool foundholes = false;
    Extract_Simples( BOOL_OR, true, foundholes );
    Split( Result );

    //Result contains the separate boundaries and holes

    //ring creation should never be alternate rule, since it overlaps.
    //So temprarely modify it.
    bool rule = _GC->GetWindingRule();
    _GC->SetWindingRule( true );

    _GC->SetState( "Create rings" );
    //first create a ring around all simple graphs
    {
        TDLI<kbGraph> IResult = TDLI<kbGraph>( Result );
        kbGraphList *_ring = new kbGraphList( _GC );
        {
            //put into one graphlist
            IResult.tohead();
            int i;
            int n = IResult.count();
            for ( i = 0;i < n;i++ )
            {
                {
                    IResult.item()->MakeClockWise();
                    IResult.item()->CreateRing_fast( _ring, fabs( factor ) );
                    //     IResult.item()->CreateRing(_ring,fabs(factor));
                }
                delete( IResult.item() );
                IResult.remove();

                //move ring graphlist to result
                while ( !_ring->empty() )
                {
                    //add to end
                    ( ( kbGraph* )_ring->headitem() )->MakeClockWise();
                    IResult.insend( ( kbGraph* )_ring->headitem() );
                    _ring->removehead();
                }
            }
        }
        delete _ring;

        //IResult contains all rings
        //prepare the graphs for extracting the links of a certain operation
        //set original graphlist to groupA and ring to groupB
        int i = 2;
        IResult.tohead();
        while ( !IResult.hitroot() )
        {
            IResult.item()->Reset_flags();
            IResult.item()->SetGroup( GROUP_B );
            IResult.item()->SetNumber( i );
            i++;
            IResult++;
        }
    }

    //a ring shape can overlap itself, for alternate filling this is problem.
    //doing a merge in winding rule makes this oke, since overlap is removed by it.
    if ( !rule ) //alternate rule?
    {
        Prepare( 1 );
        Boolean( BOOL_OR, Result );

        TDLI<kbGraph> IResult = TDLI<kbGraph>( Result );
        //IResult contains all rings
        //prepare the graphs for extracting the links of a certain operation
        //set original graphlist to groupA and ring to groupB
        int i = 2;
        IResult.tohead();
        while ( !IResult.hitroot() )
        {
            IResult.item()->Reset_flags();
            IResult.item()->SetGroup( GROUP_B );
            IResult.item()->SetNumber( i );
            i++;
            IResult++;
        }
    }

    //restore filling rule
    _GC->SetWindingRule( rule );

    TakeOver( original );
    Reset_flags();
    SetNumber( 1 );
    SetGroup( GROUP_A );
    Result->MakeOneGraph( this ); // adds all graph its links to original
    // Result will be empty afterwords


    //merge ring with original shapes for positive correction else subtract ring

    //calculate intersections etc.
    //SINCE correction will calculate intersections between
    //ring and original _GC->Get_Marge() must be set a lot smaller then factor
    //during the rest of this routine
    //else wierd effects will be the result.
    double Backup_Marge = _GC->GetMarge();

    if ( _GC->GetInternalMarge() > fabs( factor / 100 ) )
    {
        _GC->SetInternalMarge( ( B_INT ) fabs( factor / 100 ) );
        //less then 1 is usless since all coordinates are integers
        if ( _GC->GetInternalMarge() < 1 )
            _GC->SetInternalMarge( 1 );
    }


    Prepare( 1 );

    _GC->SetState( "Add/Substract rings" );

    if ( factor > 0 )
        Boolean( BOOL_OR, Result );
    else
        Boolean( BOOL_A_SUB_B, Result );

    _GC->SetMarge( Backup_Marge );

    //the result of the graph correction is in Result
    delete original;
}

// Perform an operation on the graph
void kbGraph::MakeRing( kbGraphList* Result, double factor )
{

    bool rule = _GC->GetWindingRule();
    _GC->SetWindingRule( true );

    // At this moment we have one graph
    // step one, split it up in single graphs, and mark the holes
    // step two, make one graph again and link the holes
    // step three, split up again and dump the result in Result
    _GC->SetState( "Extract simple graphs" );

    //extract the (MERGE or OR) result from the graph
    SetNumber( 1 );
    Prepare( 1 );

    ResetBinMark();
    //DeleteNonCond(BOOL_OR);
    HandleNonCond( BOOL_OR );

    bool foundholes = false;
    Extract_Simples( BOOL_OR, true, foundholes );
    Split( Result );
    //Iresult contains the separate boundaries and holes
    //make a correction on the boundaries factor
    //make a correction on the holes -factor

    _GC->SetState( "Create rings" );

    //first create a ring around all simple graphs
    TDLI<kbGraph> IResult = TDLI<kbGraph>( Result );
    kbGraphList *_ring = new kbGraphList( _GC );
    {
        IResult.tohead();
        int i;
        int n = IResult.count();
        for ( i = 0;i < n;i++ )
        {
            {
                IResult.item()->MakeClockWise();
                IResult.item()->CreateRing_fast( _ring, fabs( factor ) );
            }
            delete( IResult.item() );
            IResult.remove();

            //move ring graphlist to result
            while ( !_ring->empty() )
            {
                //add to end
                ( ( kbGraph* )_ring->headitem() )->MakeClockWise();
                IResult.insend( ( kbGraph* )_ring->headitem() );
                _ring->removehead();
            }
        }
    }
    delete _ring;
    _GC->SetWindingRule( rule );
}

//create a ring shapes on every edge of the graph
void kbGraph::CreateRing( kbGraphList *ring, double factor )
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.tohead();
    while( !_LI.hitroot() )
    {
        kbGraph * shape = new kbGraph( _GC );
        //generate shape around link
        shape->Make_Rounded_Shape( _LI.item(), factor );
        ring->insbegin( shape );
        _LI++;
    }
}

//create a ring shapes on every edge of the graph
void kbGraph::CreateRing_fast( kbGraphList *ring, double factor )
{
    kbNode * begin;
    kbLink* currentlink;
    kbLine  currentline( _GC );

    kbLine  firstline( _GC );

    kbLink* nextlink;
    kbLine nextline( _GC );

    {
        TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
        _LI.foreach_mf( &kbLink::UnMark );//reset bin and mark flag of each link
        _LI.mergesort( linkYXsorter );
        _LI.tohead();

        begin = GetMostTopLeft( &_LI ); // from all the most Top nodes,
        // take the most left one
    }
    if ( !begin )
        return;

    currentlink = begin->GetIncomingLink();
    currentline.Set( currentlink );
    currentline.CalculateLineParameters();

    nextlink = begin->GetOutgoingLink();
    nextline.Set( nextlink );
    nextline.CalculateLineParameters();

    firstline.Set( nextlink );
    firstline.CalculateLineParameters();

    while ( nextlink )
    {
        kbGraph * shape = new kbGraph( _GC );
        {

            kbNode* _last_ins_left  = 0;
            kbNode* _last_ins_right = 0;

            currentline.Create_Begin_Shape( &nextline, &_last_ins_left, &_last_ins_right, factor, shape );

            while( true )
            {
                currentline = nextline;
                currentlink = nextlink;
                currentlink->SetBeenHere();

                nextlink = currentlink->GetEndNode()->Follow( currentlink );
                if ( nextlink )
                {
                    nextline.Set( nextlink );
                    nextline.CalculateLineParameters();
                    if ( !currentline.Create_Ring_Shape( &nextline, &_last_ins_left, &_last_ins_right, factor, shape ) )
                        break;
                }
                else
                    break;
            }

            //finish this part
            if ( nextlink )
                currentline.Create_End_Shape( &nextline, _last_ins_left, _last_ins_right, factor, shape );
            else
                currentline.Create_End_Shape( &firstline, _last_ins_left, _last_ins_right, factor, shape );

            shape->MakeOneDirection();
            shape->MakeClockWise();
        }

        //if the shape is very small first merge it with the previous shape
        if ( !ring->empty() && shape->Small( ( B_INT ) fabs( factor * 3 ) ) )
        {
            TDLI<kbGraph> Iring = TDLI<kbGraph>( ring );

            Iring.totail();

            kbGraphList *_twoshapes = new kbGraphList( _GC );
            _twoshapes->insbegin( shape );
            _twoshapes->insbegin( Iring.item() );
            Iring.remove();
            _twoshapes->Merge();

            //move merged graphlist to ring
            Iring.takeover( _twoshapes );
            delete _twoshapes;
        }
        else
            ring->insend( shape );

        currentlink->SetBeenHere();
    }
}

//create an arc and add it to the graph
//center of circle
//begin point of arc
//end point of arc
//radius of arc
//aberation for generating the segments
void kbGraph::CreateArc( kbNode* center, kbNode* begin, kbNode* end, double radius, bool clock, double aber )
{
    double phi, dphi, dx, dy;
    int Segments;
    int i;
    double ang1, ang2, phit;

    kbNode* _last_ins;
    kbNode* _current;

    _last_ins = begin;

    dx = ( double ) _last_ins->GetX() - center->GetX();
    dy = ( double ) _last_ins->GetY() - center->GetY();
    ang1 = atan2( dy, dx );
    if ( ang1 < 0 ) ang1 += 2.0 * M_PI;
    dx = ( double ) end->GetX() - center->GetX();
    dy = ( double ) end->GetY() - center->GetY();
    ang2 = atan2( dy, dx );
    if ( ang2 < 0 ) ang2 += 2.0 * M_PI;

    if ( clock )
    { //clockwise
        if ( ang2 > ang1 )
            phit = 2.0 * M_PI - ang2 + ang1;
        else
            phit = ang1 - ang2;
    }
    else
    { //counter_clockwise
        if ( ang1 > ang2 )
            phit = -( 2.0 * M_PI - ang1 + ang2 );
        else
            phit = -( ang2 - ang1 );
    }

    //what is the delta phi to get an accurancy of aber
    dphi = 2 * acos( ( radius - aber ) / radius );

    //set the number of segments
    if ( phit > 0 )
        Segments = ( int )ceil( phit / dphi );
    else
        Segments = ( int )ceil( -phit / dphi );

    if ( Segments <= 1 )
        Segments = 1;
    if ( Segments > 6 )
        Segments = 6;

    dphi = phit / ( Segments );

    for ( i = 1; i < Segments; i++ )
    {
        dx = ( double ) _last_ins->GetX() - center->GetX();
        dy = ( double ) _last_ins->GetY() - center->GetY();
        phi = atan2( dy, dx );

        _current = new kbNode( ( B_INT ) ( center->GetX() + radius * cos( phi - dphi ) ),
                             ( B_INT ) ( center->GetY() + radius * sin( phi - dphi ) ), _GC );
        AddLink( _last_ins, _current );

        _last_ins = _current;
    }

    // make a node from the endnode of link
    AddLink( _last_ins, end );
}

void kbGraph::CreateArc( kbNode* center, kbLine* incoming, kbNode* end, double radius, double aber )
{
    double distance = 0;
    if ( incoming->PointOnLine( center, distance, _GC->GetAccur() ) == RIGHT_SIDE )
        CreateArc( center, incoming->GetEndNode(), end, radius, true, aber );
    else
        CreateArc( center, incoming->GetEndNode(), end, radius, false, aber );
}

void kbGraph::MakeOneDirection()
{
    int _nr_of_points = _linklist->count();
    kbLink* _current = ( kbLink* )_linklist->headitem();

    kbNode* _last = _current->GetBeginNode();
    kbNode* dummy;

    for ( int i = 0; i < _nr_of_points; i++ )
    {
        // get the other node on the link
        _last = _current->GetOther( _last );
        // get the other link on the node
        _current = _current->Forth( _last );

        if ( _current->GetBeginNode() != _last )
        {
            // swap the begin- and endnode of the current link
            dummy = _current->GetBeginNode();
            _current->SetBeginNode( _current->GetEndNode() );
            _current->SetEndNode( dummy );
        }
    }
}

bool kbGraph::Small( B_INT howsmall )
{

    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    _LI.tohead();
    kbNode* bg = _LI.item()->GetBeginNode();
    B_INT xmin = bg->GetX();
    B_INT xmax = bg->GetX();
    B_INT ymin = bg->GetY();
    B_INT ymax = bg ->GetY();
    while ( !_LI.hitroot() )
    {
        bg = _LI.item()->GetBeginNode();
        // make _boundingbox bigger if the link makes the graph bigger
        // Checking if point is in bounding-box with marge
        xmin = bmin( xmin, bg->GetX() );
        xmax = bmax( xmax, bg->GetX() );
        ymin = bmin( ymin, bg->GetY() );
        ymax = bmax( ymax, bg->GetY() );
        _LI++;
    }

    B_INT dx = ( xmax - xmin );
    B_INT dy = ( ymax - ymin );

    if ( ( dx < howsmall ) && ( dy < howsmall ) )
        return true;

    return false;
}


//create a circle at end and begin point
// and block in between
void kbGraph::Make_Rounded_Shape( kbLink* a_link, double factor )
{
    double phi, dphi, dx, dy;
    int Segments = 6;
    int i;


    kbLine theline( a_link, _GC );
    theline.CalculateLineParameters();

    kbNode* _current;
    kbNode *_first = new kbNode( a_link->GetBeginNode(), _GC );
    kbNode *_last_ins = _first;

    theline.Virtual_Point( _first, factor );

    // make a node from this point
    _current = new kbNode( a_link->GetEndNode(), _GC );
    theline.Virtual_Point( _current, factor );

    // make a link between the current and the previous and add this to graph
    AddLink( _last_ins, _current );
    _last_ins = _current;

    // make a half circle around the clock with the opposite point as
    // the middle point of the circle
    dphi = M_PI / ( Segments );
    for ( i = 1; i < Segments; i++ )
    {
        dx = ( double ) _last_ins->GetX() - a_link->GetEndNode()->GetX();
        dy = ( double ) _last_ins->GetY() - a_link->GetEndNode()->GetY();
        phi = atan2( dy, dx );

        _current = new kbNode( ( B_INT ) ( a_link->GetEndNode()->GetX() + factor * cos( phi - dphi ) ),
                             ( B_INT ) ( a_link->GetEndNode()->GetY() + factor * sin( phi - dphi ) ), _GC );

        AddLink( _last_ins, _current );

        _last_ins = _current;
    }

    // make a node from the endnode of link
    _current = new kbNode( a_link->GetEndNode(), _GC );
    theline.Virtual_Point( _current, -factor );
    AddLink( _last_ins, _current );
    _last_ins = _current;

    // make a node from this beginnode of link
    _current = new kbNode( a_link->GetBeginNode(), _GC );
    theline.Virtual_Point( _current, -factor );
    AddLink( _last_ins, _current );
    _last_ins = _current;

    for ( i = 1; i < Segments; i++ )
    {
        dx = ( double ) _last_ins->GetX() - a_link->GetBeginNode()->GetX();
        dy = ( double ) _last_ins->GetY() - a_link->GetBeginNode()->GetY();
        phi = atan2( dy, dx );

        _current = new kbNode( ( B_INT )( a_link->GetBeginNode()->GetX() + factor * cos( phi - dphi ) ),
                             ( B_INT )( a_link->GetBeginNode()->GetY() + factor * sin( phi - dphi ) ), _GC );

        AddLink( _last_ins, _current );

        _last_ins = _current;
    }

    // make a link between the last and the first to close the graph
    AddLink( _last_ins, _first );
};

//make the graph clockwise orientation,
//return if the graph needed redirection
bool kbGraph::MakeClockWise()
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    if ( _LI.empty() ) return false;

    kbLink *currentlink;
    kbNode *begin;

    _LI.foreach_mf( &kbLink::UnMark );//reset bin and mark flag of each link
    _LI.mergesort( linkYXtopsorter );
    _LI.tohead();

    begin = GetMostTopLeft( &_LI ); // from all the most Top nodes,
    // take the most left one

    currentlink = begin->GetNotFlat();
    if ( !currentlink )
    {
        char buf[100];
        sprintf( buf, "no NON flat link MakeClockWise at %15.3lf , %15.3lf",
                 double( begin->GetX() ), double( begin->GetY() ) );
        throw kbBool_Engine_Error( buf, "Error", 9, 0 );
    }

    //test to see if the orientation is right or left
    if ( currentlink->GetBeginNode() == begin )
    {
        if ( currentlink->GetEndNode()->GetX() < begin->GetX() )
        {
            //going left
            //it needs redirection
            ReverseAllLinks();
            return true;
        }
    }
    else
    {
        if ( currentlink->GetBeginNode()->GetX() > begin->GetX() )
        {  //going right
            //it needs redirection
            ReverseAllLinks();
            return true;
        }
    }
    return false;
}

//make the graph clockwise orientation,
//return if the graph needed redirection
bool kbGraph::MakeCounterClockWise()
{
    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    if ( _LI.empty() ) return false;

    kbLink *currentlink;
    kbNode *begin;

    _LI.foreach_mf( &kbLink::UnMark );//reset bin and mark flag of each link
    _LI.mergesort( linkYXtopsorter );
    _LI.tohead();

    begin = GetMostTopLeft( &_LI ); // from all the most Top nodes,
    // take the most left one

    currentlink = begin->GetNotFlat();
    if ( !currentlink )
    {
        char buf[100];
        sprintf( buf, "no NON flat link MakeCounterClockWise at %15.3lf , %15.3lf",
                 double( begin->GetX() ), double( begin->GetY() ) );
        throw kbBool_Engine_Error( buf, "Error", 9, 0 );
    }

    //test to see if the orientation is right or left
    if ( currentlink->GetBeginNode() == begin )
    {
        if ( currentlink->GetEndNode()->GetX() > begin->GetX() )
        {
            //going left
            //it needs redirection
            ReverseAllLinks();
            return true;
        }
    }
    else
    {
        if ( currentlink->GetBeginNode()->GetX() < begin->GetX() )
        {  //going right
            //it needs redirection
            ReverseAllLinks();
            return true;
        }
    }
    return false;
}

bool kbGraph::writegraph( bool linked )
{
    if ( !_GC->GetDebug() )
        return true;

    FILE * file = _GC->GetLogFile();

    if ( file == NULL )
        return true;

    fprintf( file, "# graph\n" );

    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    if ( _LI.empty() )
    {
        return true;
    }

    _LI.tohead();
    while( !_LI.hitroot() )
    {
        kbLink * curl = _LI.item();

        fprintf( file, " linkbegin %I64d %I64d \n", curl->GetBeginNode()->GetX() , curl->GetBeginNode()->GetY() );

        if ( linked )
        {
            TDLI<kbLink> Inode( curl->GetBeginNode()->GetLinklist() );
            Inode.tohead();
            while( !Inode.hitroot() )
            {

                fprintf( file, " b %I64d %I64d \n", Inode.item()->GetBeginNode()->GetX() , Inode.item()->GetBeginNode()->GetY() );

                fprintf( file, " e %I64d %I64d \n", Inode.item()->GetEndNode()->GetX() , Inode.item()->GetEndNode()->GetY() );

                Inode++;
            }
        }
        fprintf( file, " linkend %I64d %I64d \n", curl->GetEndNode()->GetX() , curl->GetEndNode()->GetY() );


        if ( linked )
        {
            TDLI<kbLink> Inode( curl->GetEndNode()->GetLinklist() );
            Inode.tohead();
            while( !Inode.hitroot() )
            {

                fprintf( file, " b %I64d %I64d \n", Inode.item()->GetBeginNode()->GetX() , Inode.item()->GetBeginNode()->GetY() );

                fprintf( file, " e %I64d %I64d \n", Inode.item()->GetEndNode()->GetX() , Inode.item()->GetEndNode()->GetY() );

                Inode++;
            }

        }

        if ( curl->GetBeginNode() == curl->GetEndNode() )
            fprintf( file, "     null_link \n" );

        fprintf( file, "    group %d ", curl->Group() );
        fprintf( file, "    bin   %d ", curl->BeenHere() );
        fprintf( file, "    mark  %d ", curl->IsMarked() );
        fprintf( file, "    leftA %d ", curl->GetLeftA() );
        fprintf( file, "    rightA %d ", curl->GetRightA() );
        fprintf( file, "    leftB %d ", curl->GetLeftB() );
        fprintf( file, "    rightB %d \n", curl->GetRightB() );
        fprintf( file, "    or %d ", curl->IsMarked( BOOL_OR ) );
        fprintf( file, "    and %d " , curl->IsMarked( BOOL_AND ) );
        fprintf( file, "    exor %d " , curl->IsMarked( BOOL_EXOR ) );
        fprintf( file, "    a_sub_b %d " , curl->IsMarked( BOOL_A_SUB_B ) );
        fprintf( file, "    b_sub_a %d " , curl->IsMarked( BOOL_B_SUB_A ) );
        fprintf( file, "    hole %d " , curl->GetHole() );
        fprintf( file, "    top_hole %d \n" , curl->IsTopHole() );
        fprintf( file, "    arcPiece %d \n" , curl->GetArcPiece() );

        _LI++;
    }

    return true;
}

bool kbGraph::writeintersections()
{

    if ( !_GC->GetDebug() )
        return true;

    FILE * file = _GC->GetLogFile();

    if ( file == NULL )
        return true;

    fprintf( file, "# graph\n" );

    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    if ( _LI.empty() )
    {
        return true;
    }

    _LI.tohead();
    while( !_LI.hitroot() )
    {
        kbLink * curl = _LI.item();
        TDLI<kbLink> Inode( curl->GetBeginNode()->GetLinklist() );
        Inode.tohead();
        if ( Inode.count() > 2 )
        {
            fprintf( file, " count %I64d", Inode.count() );
            fprintf( file, " b %I64d %I64d \n\n", curl->GetBeginNode()->GetX() , curl->GetBeginNode()->GetY() );
        }
        _LI++;
    }

    return true;
}

bool kbGraph::checksort()
{
    // if empty then just insert
    if ( _linklist->empty() )
        return true;

    TDLI<kbLink> _LI = TDLI<kbLink>( _linklist );
    // put new item left of the one that is bigger
    _LI.tohead();
    kbLink* prev = _LI.item();
    kbLink* cur = _LI.item();
    _LI++;
    while( !_LI.hitroot() )
    {
        kbLink * aap = _LI.item();
        if ( linkXYsorter( prev, _LI.item() ) == -1 )
        {
            cur = aap;

            return false;
        }
        prev = _LI.item();
        _LI++;
    }
    return true;
}


void kbGraph::WriteKEY( kbBool_Engine* GC, FILE* file )
{
    double scale = 1.0 / GC->GetGrid() / GC->GetGrid();

    bool ownfile = false;
    if ( !file )
    {
        file = fopen( "keyfile.key", "w" );
        ownfile = true;

        fprintf( file, "\
                 HEADER 5; \
                 BGNLIB; \
                 LASTMOD {2-11-15  15:39:21}; \
                 LASTACC {2-11-15  15:39:21}; \
                 LIBNAME trial; \
                 UNITS; \
                 USERUNITS 0.0001; PHYSUNITS 1e-009; \
                 \
                 BGNSTR;  \
                 CREATION {2-11-15  15:39:21}; \
                 LASTMOD  {2-11-15  15:39:21}; \
                 STRNAME top; \
                 ");
    }

    TDLI<kbLink> _LI=TDLI<kbLink>(_linklist);
    if (_LI.empty())
    {
        if ( ownfile )
        {
            fprintf(file,"\
                    ENDSTR top; \
                    ENDLIB; \
                    ");
            fclose (file);

        }
        return;
    }


    _LI.tohead();
    kbLink* curl = _LI.item();

    if ( _LI.item()->Group() == GROUP_A )
        fprintf(file,"BOUNDARY; LAYER 0;  DATATYPE 0;\n");
    else
        fprintf(file,"BOUNDARY; LAYER 1;  DATATYPE 0;\n");
    fprintf(file," XY % d; \n", _LI.count()+1 );

    double firstx = curl->GetBeginNode()->GetX()*scale;
    double firsty = curl->GetBeginNode()->GetY()*scale;
    fprintf(file,"X % f;\t", firstx);
    fprintf(file,"Y % f; \n", firsty);
    _LI++;

    while(!_LI.hitroot())
    {
        kbLink* curl = _LI.item();

        fprintf(file,"X % f;\t", curl->GetBeginNode()->GetX()*scale);
        fprintf(file,"Y % f; \n", curl->GetBeginNode()->GetY()*scale);

        _LI++;
    }
    fprintf(file,"X % f;\t", firstx);
    fprintf(file,"Y % f; \n", firsty);
    fprintf(file,"ENDEL;\n");

    if ( ownfile )
    {
        fprintf(file,"\
                ENDSTR top; \
                ENDLIB; \
                ");
        fclose (file);

    }
}


void kbGraph::WriteGraphKEY( kbBool_Engine* GC, const string& filename )
{

    double scale = 1.0/GC->GetGrid()/GC->GetGrid();

    FILE* file = fopen( filename.c_str(), "w");

    fprintf(file,"\
            HEADER 5; \
            BGNLIB; \
            LASTMOD {2 - 11 - 15  15: 39: 21}; \
            LASTACC {2 - 11 - 15  15: 39: 21}; \
            LIBNAME trial; \
            UNITS; \
            USERUNITS 1; PHYSUNITS 1e-006; \
            \
            BGNSTR;  \
            CREATION {2 - 11 - 15  15: 39: 21}; \
            LASTMOD  {2 - 11 - 15  15: 39: 21}; \
            STRNAME top; \
            ");


    TDLI<kbLink> _LI=TDLI<kbLink>(_linklist);
    if (_LI.empty())
    {
        fprintf(file,"\
                ENDSTR top; \
                ENDLIB; \
                ");
        fclose (file);
        return;
    }

    _LI.tohead();
    kbLink* curl;

    int _nr_of_points = _linklist->count();
    for (int i = 0; i < _nr_of_points; i++)
    {
        curl = _LI.item();

        if ( curl->Group() == GROUP_A )
            fprintf(file,"PATH; LAYER 0;\n");
        else
            fprintf(file,"PATH; LAYER 1;\n");

        fprintf(file," XY % d; \n", 2 );
        fprintf(file,"X % f;\t", curl->GetBeginNode()->GetX()*scale);
        fprintf(file,"Y % f; \n", curl->GetBeginNode()->GetY()*scale);
        fprintf(file,"X % f;\t", curl->GetEndNode()->GetX()*scale);
        fprintf(file,"Y % f; \n", curl->GetEndNode()->GetY()*scale);
        _LI++;
        fprintf(file,"ENDEL;\n");
    }

    fprintf(file,"\
            ENDSTR top; \
            ENDLIB; \
            ");

    fclose (file);
}




