/*! \file src/scanbeam.cpp
    \author Klaas Holwerda or Julian Smart
 
    Copyright: 2001-2004 (C) Klaas Holwerda 
 
    Licence: see kboollicense.txt 
 
    RCS-ID: $Id: scanbeam.cpp,v 1.7 2009/09/14 16:50:12 titato Exp $
*/

// class scanbeam
// this class represents de space between two scanlines
#include "kbool/scanbeam.h"
#include <math.h>
#include <assert.h>

#include "kbool/booleng.h"

#include "kbool/graph.h"
#include "kbool/node.h"

//this here is to initialize the static iterator of scanbeam
//with NOLIST constructor

int recordsorter( kbRecord* , kbRecord* );

int recordsorter_ysp_angle( kbRecord* , kbRecord* );
int recordsorter_ysp_angle_back( kbRecord* rec1, kbRecord* rec2 );

ScanBeam::ScanBeam( kbBool_Engine* GC ): DL_List<kbRecord*>()
{
    _GC = GC;
    _type = NORMAL;
    _BI.Attach( this );
}

ScanBeam::~ScanBeam()
{
    //first delete all record still in the beam
    _BI.Detach();
    remove_all( true );

    //DeleteRecordPool();
}

void ScanBeam::SetType( kbNode* low, kbNode* high )
{
    if ( low->GetX() < high->GetX() )
        _type = NORMAL;
    else
        _type = FLAT;
}

/*
//catch node to link crossings
// must be sorted on ysp
int ScanBeam::FindCloseLinksAndCross(TDLI<kbLink>* _I,kbNode* _lowf)
{
 int merges = 0;
 kbRecord* record;
 
   TDLI<kbRecord> _BBI=TDLI<kbRecord>(this);
 
 if (_BI.count() > 1)
 {
  //first search a link towards this node
  for(_BI.tohead(); !_BI.hitroot(); _BI++)
  {
   record=_BI.item();
   if( (record->GetLink()->GetBeginNode()==_lowf) ||
     (record->GetLink()->GetEndNode()  ==_lowf)
     )
     break;
  }
 
  //NOTICE if the node "a_node" is not inside a record
  //for instance to connected flat links (flatlinks not in beam)
  //then IL will be at end    (those will be catched at 90 degrees rotation)
  if (_BI.hitroot())
      {
   return(merges);
      }
 
      //from IL search back for close links
      _BBI.toiter(&_BI);
      _BBI--;
      while(!_BBI.hitroot())
      {
         record=_BBI.item();
 
         if (record->Ysp() != _lowf->GetY())
            break;
 
         // the distance to the low node is smaller then the MARGE
         if( (record->GetLink()->GetBeginNode()!=_lowf) &&
             (record->GetLink()->GetEndNode()  !=_lowf)
           )
         {  // the link is not towards the low node
            record->GetLink()->AddCrossing(_lowf);
            record->SetNewLink(record->GetLink()->ProcessCrossingsSmart(_I));
            merges++;
         }
         _BBI--;
      }
 
      //from IL search forward for close links
      _BBI.toiter(&_BI);
      _BBI++;
      while(!_BBI.hitroot())
      {
         record=_BBI.item();
 
         if (record->Ysp() != _lowf->GetY())
//         if (record->Ysp() < _lowf->GetY()-MARGE)
            break;
 
         // the distance to the low node is smaller then the MARGE
         if( (record->GetLink()->GetBeginNode()!=_lowf) &&
             (record->GetLink()->GetEndNode()  !=_lowf)
           )
         {  // the link is not towards the low node
            record->GetLink()->AddCrossing(_lowf);
            record->SetNewLink(record->GetLink()->ProcessCrossingsSmart(_I));
            merges++;
         }
         _BBI++;
      }
   }
 return merges;
}
*/

/*
bool  ScanBeam::Update(TDLI<kbLink>* _I,kbNode* _lowf)
{
 bool found=false;
 kbLink* link;
 
 _BI.tohead();
 while (!_BI.hitroot())
 {
 
  kbRecord* record=_BI.item();
  record->Calc_Ysp(_type,_low);
  _BI++;
 }
 
   FindCloseLinksAndCross(_I,_lowf);
 
   _BI.tohead();
   while (!_BI.hitroot())
   {
      kbRecord* record=_BI.item();
      //records containing links towards the new low node
      //are links to be marked for removal
 
      if ((record->GetLink()->GetEndNode() == _lowf) ||
          (record->GetLink()->GetBeginNode() == _lowf)
         )
      {
         //cross here the links that meat eachother now
         delete _BI.item();
         _BI.remove();
 
         //cross here the links that meat eachother now
         _BI--;
         if (!_BI.hitroot() && (_BI.count() > 1))
         {
            kbRecord* prev=_BI.item();
            _BI++;
            if (!_BI.hitroot())
            {
               if (!_BI.item()->Equal(prev)) // records NOT parallel
                  if (_BI.item()->GetLine()->Intersect(prev->GetLine(),MARGE))
                  {
                     //they did cross, integrate the crossings in the graph
                     //this  may modify the links already part of the record
                     //this is why they are returned in set for the record
                     _BI.item()->SetNewLink(_BI.item()->GetLink()->ProcessCrossingsSmart(_I));
                     prev->SetNewLink(prev->GetLink()->ProcessCrossingsSmart(_I));
                  }
            }
         }
         else
           _BI++;
      }
      else
         _BI++;
   }
 
 //writebeam();
 
   //ONLY links towards the low node are possible to be added
 //the bin flag will be set if it fits in the beam
 //so for following beams it will not be checked again
 while ( bool(link=_lowf->GetBinHighest(false)) )
 {
      kbRecord* record=new kbRecord(link);
      // yp_new will always be the y of low node since all new links are
      // from this node
      record->SetYsp(_lowf->GetY());
      record->Set_Flags(_type);
      //need to calculate ysn to be able to sort this record in the right order
      //this is only used when the insert node is equal for both records
      // ins_smart and cross neighbour  directly
      // if empty then just insert
 
      if (empty())
         insend(record);
      else
      {
         // put new item left of the one that is bigger
         _BI.tohead();
         while(!_BI.hitroot())
         {
            if (recordsorter_ysp_angle(record,_BI.item())==1)
               break;
            _BI++;
         }
 
         _BI.insbefore(record);
         _BI--;_BI--; //just before the new record inserted
         if (!_BI.hitroot())
         {
            kbRecord* prev=_BI.item();
            _BI++; //goto the new record inserted
            if (!_BI.item()->Equal(prev)) // records NOT parallel
            {
               if (_BI.item()->GetLine()->Intersect(prev->GetLine(),MARGE))
               {
                  //this  may modify the links already part of the record
                  //this is why they are returned in set for the record
                  _BI.item()->SetNewLink(_BI.item()->GetLink()->ProcessCrossingsSmart(_I));
                  prev->SetNewLink(prev->GetLink()->ProcessCrossingsSmart(_I));
               }
            }
         }
         else
           _BI++;
 
         kbRecord* prev=_BI.item(); //the new record
         _BI++;
         if (!_BI.hitroot() && !_BI.item()->Equal(prev)) // records NOT parallel
         {
          kbRecord* cur=_BI.item();
            if (cur->GetLine()->Intersect(prev->GetLine(),MARGE))
            {
               //this  may modify the links already part of the record
               //this is why they are returned in set for the record
               cur->SetNewLink(cur->GetLink()->ProcessCrossingsSmart(_I));
               prev->SetNewLink(prev->GetLink()->ProcessCrossingsSmart(_I));
            }
         }
      }
      //remember this to calculate in/out values for each new link its polygon again.
      GNI->insend(record->GetLink()->GetGraphNum());
      found=true;
    record->GetLink()->SetBeenHere();
   }
 
   FindCloseLinksAndCross(_I,_lowf);
 //writebeam();
 return(found);
}
*/

bool ScanBeam::FindNew( SCANTYPE scantype, TDLI<kbLink>* _I, bool& holes )
{
    bool foundnew = false;

    _low = _I->item()->GetBeginNode();

    kbLink* link;

    //if (!checksort())
    // SortTheBeam();

    lastinserted = 0;
    //ONLY links towards the low node are possible to be added
    //the bin flag will be set if it fits in the beam
    //so for following beams it will not be checked again
    while ( ( link = _low->GetBinHighest( false ) ) != NULL )
    {
        if ( ( link->GetEndNode()->GetX() == link->GetBeginNode()->GetX() ) //flatlink in flatbeam
                && ( ( scantype == NODELINK ) || ( scantype == LINKLINK ) || ( scantype == LINKHOLES ) )
           )
        {
            switch( scantype )
            {
                case NODELINK:
                {
                    //all vertical links in flatbeam are ignored
                    //normal link in beam
                    kbRecord * record = new kbRecord( link, _GC );
                    // yp_new will always be the y of low node since all new links are
                    // from this node
                    record->SetYsp( _low->GetY() );
                    record->Set_Flags();
                    // put new item left of the one that is lower in the beam
                    // The last one inserted in this loop, is already left of the current
                    // iterator position. So the new links are inerted in proper order.
                    link->SetRecordNode( _BI.insbefore( record ) );
                    _BI--;
                    foundnew = Process_PointToLink_Crossings() != 0 || foundnew;
                    delete record;
                    _BI.remove();
                    break;
                }
                case LINKLINK:
                    //is the new record a flat link
                {
                    kbLine flatline = kbLine( link, _GC );
                    foundnew = Process_LinkToLink_Flat( &flatline ) || foundnew;
                    //flatlinks are not part of the beams, still they are used to find new beams
                    //they can be processed now if the beginnode does not change, since this is used to
                    //to find new beams. and its position does not change
                    //ProcessCrossings does take care of this
                    flatline.GetLink()->ProcessCrossings( _I );
                    break;
                }
                case LINKHOLES : //holes are never to flatlinks
                    assert( true );
                default:
                    break;
            }
        }
        else
        {
            //normal link in beam
            kbRecord* record = new kbRecord( link, _GC );
            // yp_new will always be the y of low node since all new links are
            // from this node
            record->SetYsp( _low->GetY() );
            record->Set_Flags();
            // put new item left of the one that is lower in the beam
            // The last one inserted in this loop, is already left of the current
            // iterator position. So the new links are inserted in proper order.
            link->SetRecordNode( _BI.insbefore( record ) );
            lastinserted++;

            _GC->Write_Log( "after insert" );
            writebeam();

            switch( scantype )
            {
                case NODELINK:
                    _BI--;
                    foundnew = Process_PointToLink_Crossings() != 0  || foundnew;
                    _BI++;
                    break;
                case INOUT:
                {
                    _BI--;
                    //now we can set the _inc flag
                    Generate_INOUT( record->GetLink()->GetGraphNum() );
                    _BI++;
                }
                break;
                case GENLR:
                {
                    //now we can set the a/b group flags based on the above link
                    _BI--;
                    _BI--;
                    kbRecord* above = 0;
                    if ( !_BI.hitroot() )
                        above = _BI.item();
                    _BI++;

                    //something to do for winding rule

                    if ( record->Calc_Left_Right( above ) )
                    {
                        delete record;
                        _BI.remove();
                        lastinserted--;
                    }
                    else
                        _BI++;
                }
                break;
                case LINKHOLES:
                    _BI--;
                    holes = ProcessHoles( true, _I ) || holes;
                    _BI++;
                    break;

                default:
                    break;
            }
        }
        link->SetBeenHere();
    }

    writebeam();

    return foundnew;
}

bool ScanBeam::RemoveOld( SCANTYPE scantype, TDLI<kbLink>* _I, bool& holes )
{
    bool found = false;
    bool foundnew = false;
    DL_Iter<kbRecord*>  _BBI = DL_Iter<kbRecord*>();
    bool attached = false;

    _low = _I->item()->GetBeginNode();

    switch( scantype )
    {
        case INOUT:
        case GENLR:
        case LINKHOLES:
            if ( _type == NORMAL )
            {
                if ( _low->GetBinHighest( true ) ) //is there something to remove
                {
                    if ( scantype == LINKHOLES )
                    {
                        // Tophole links can be linked at the begin or end point, depending on
                        // which is higher in Y.
                        // A link pointing to the low node, and which is a tophole link,
                        // and which was not linked in sofar should be linked now.
                        _BI.tohead();
                        while ( !_BI.hitroot() )
                        {
                            kbRecord * record = _BI.item();
                            //records containing links towards the new low node
                            //are links to be removed
                            if ( ( record->GetLink()->GetEndNode() == _low ) ||
                                    ( record->GetLink()->GetBeginNode() == _low )
                               )
                            {
                                holes = ProcessHoles( false, _I ) || holes;
                            }
                            _BI++;
                        }
                    }

                    _BI.tohead();
                    while ( !_BI.hitroot() )
                    {
                        kbRecord * record = _BI.item();
                        //records containing links towards the new low node
                        //are links to be removed
                        if ( ( record->GetLink()->GetEndNode() == _low ) ||
                                ( record->GetLink()->GetBeginNode() == _low )
                           )
                        {
                            delete _BI.item();
                            _BI.remove();
                            found = true;
                        }
                        else
                        {
                            //recalculate ysp for the new scanline
                            record->Calc_Ysp( _low );
                            _BI++;
                        }
                    }

                    // all records are renewed in Ysp.
                    // found links to remove, we search the new insert position for new links.
                    if ( found )
                    {
                        _BI.tohead();
                        while ( !_BI.hitroot() )
                        {
                            kbRecord * record = _BI.item();
                            if (  record->Ysp() < _low->GetY() )
                            {
                                break;
                            }
                            _BI++;
                        }
                    }
                }
                else
                {
                    // nothing is removed from the beam, still we moved forward with the scanline
                    // at the new _low, so we need to recalculate the intersections of the links 
                    // with the new scanline.
                    // Also the the insert position for new links is determined, being the first
                    // link below _low.
                    _BBI.Attach( this );
                    _BBI.toroot();
                    _BI.tohead();
                    while ( !_BI.hitroot() )
                    {
                        kbRecord * record = _BI.item();

                        record->Calc_Ysp( _low );
                        if ( !found && ( record->Ysp() < _low->GetY() ) )
                        {
                            found = true;
                            _BBI.toiter( &_BI );
                        }
                        _BI++;
                    }
                    _BI.toiter( &_BBI );
                    _BBI.Detach();
                }
            }
            else // _type == NORMAL
            {  //because the previous beam was flat the links to remove are
                //below the last insert position
                if ( _low->GetBinHighest( true ) ) //is there something to remove
                {

                    if ( scantype == LINKHOLES )
                    {
                        _BI.tohead();
                        while ( !_BI.hitroot() )
                        {
                            kbRecord * record = _BI.item();
                            //records containing links towards the new low node
                            //are links to be removed
                            if ( ( record->GetLink()->GetEndNode() == _low ) ||
                                    ( record->GetLink()->GetBeginNode() == _low )
                               )
                            {
                                holes = ProcessHoles( false, _I ) || holes;
                            }
                            _BI++;
                        }
                    }

                    //on record back bring us to the last inserted record
                    //or if nothing was inserted the record before the last deleted record
                    //if there was no record before the last deleted record this means
                    //we where at the beginning of the beam, so at root

                    _BI.tohead();
                    while ( !_BI.hitroot() )
                    {
                        kbRecord * record = _BI.item();
                        //records containing links towards the new low node
                        //are links to be removed
                        if ( ( record->GetLink()->GetEndNode() == _low ) ||
                                ( record->GetLink()->GetBeginNode() == _low )
                           )
                        {
                            delete _BI.item();
                            _BI.remove();
                            found = true;
                        }
                        else if ( found ) //only once in here
                            break;
                        else if ( record->Ysp() < _low->GetY() )
                            //if flatlinks are not in the beam nothing will be found
                            //this will bring us to the right insertion point
                            break;
                        else
                            _BI++;
                    }
                }
                else
                {
                    //on record back bring us to the last inserted record
                    //or if nothing was inserted the record before the last deleted record
                    //if there was no record before the last deleted record this means
                    //we where at the beginning of the beam, so at root

                    _BI.tohead();
                    while ( !_BI.hitroot() )
                    {
                        kbRecord * record = _BI.item();
                        if ( record->Ysp() < _low->GetY() )
                            break;
                        _BI++;
                    }
                }
            }
            break;

        case NODELINK:
        case LINKLINK:
        {
            if ( _type == NORMAL )
            {
                Calc_Ysp();
                if ( scantype == LINKLINK )
                    foundnew = Process_LinkToLink_Crossings() != 0 || foundnew;
                else
                    SortTheBeam( false );
            }
            //else beam is already sorted because the added/removed flat links
            //do not change the ysp of links already there, new non flat links
            //are inserted in order, as result the beam stays sorted

            if ( _low->GetBinHighest( true ) ) //is there something to remove
            {
                _BI.tohead();
                while ( !_BI.hitroot() )
                {
                    kbRecord * record = _BI.item();
                    //records containing links towards the new low node
                    //are links to be removed
                    if ( ( record->GetLink()->GetEndNode() == _low ) ||
                            ( record->GetLink()->GetBeginNode() == _low )
                       )
                    {
                        kbLine * line = record->GetLine();
                        if ( scantype == NODELINK )
                            foundnew = Process_PointToLink_Crossings() != 0 || foundnew;
                        line->GetLink()->ProcessCrossings( _I );
                        delete _BI.item();
                        _BI.remove();
                        found = true;
                    }
                    //because the beam is sorted on ysp, stop when nothing can be there to remove
                    //and the right insertion point for new links has been found
                    else if ( ( record->Ysp() < _low->GetY() ) )
                        break;
                    else
                        _BI++;
                }
            }
            else
            {
                _BI.tohead();
                while ( !_BI.hitroot() )
                {
                    kbRecord * record = _BI.item();
                    //because the beam is sorted on ysp, stop when
                    //the right insertion point for new links has been found
                    if ( ( record->Ysp() < _low->GetY() ) )
                        break;
                    _BI++;
                }
            }
        }
        break;

        default:
            break;
    }

    return foundnew;
}
/*
bool ScanBeam::RemoveOld(SCANTYPE scantype,TDLI<kbLink>* _I, bool& holes )
{
 bool found = false;
 bool foundnew = false;
   DL_Iter<kbRecord*>  _BBI=DL_Iter<kbRecord*>();
 bool attached=false;
 
   _low = _I->item()->GetBeginNode();
 
   switch(scantype)
   {
      case INOUT:
      case GENLR:
      case LINKHOLES:
      if (_type==NORMAL )
      {
         kbLink* link = _low->GetBinHighest(true);
         if ( link ) //is there something to remove
         {
            link->SetRecordNode( NULL );
 
            if ( scantype == LINKHOLES )
            {
               _BI.tohead();
               while (!_BI.hitroot())
               {
                  kbRecord* record = _BI.item();
                  //records containing links towards the new low node
                  //are links to be removed
                  if ((record->GetLink()->GetEndNode() == _low) ||
                      (record->GetLink()->GetBeginNode() == _low)
                     )
                  {
                     holes = ProcessHoles(false,_I) || holes;
                  }
                  _BI++;
               }
            }
 
            _BI.tohead();
            while (!_BI.hitroot())
            {
               kbRecord* record=_BI.item();
               //records containing links towards the new low node
               //are links to be removed
               if ((record->GetLink()->GetEndNode() == _low) ||
                   (record->GetLink()->GetBeginNode() == _low)
                  )
               {
                  delete _BI.item();
                  _BI.remove();
                  found=true;
               }
               else if (found) //only once in here
               {
                  attached=true;
                  found=false;
                  //recalculate ysp for the new scanline
                  record->Calc_Ysp(_low);
                  _BI++;
               }
               else
               {
                  //recalculate ysp for the new scanline
                  record->Calc_Ysp(_low);
                  _BI++;
               }
            }
         }
         else
         {
            _BI.tohead();
            while (!_BI.hitroot())
            {
                  kbRecord* record=_BI.item();
                  record->Calc_Ysp(_low);
                  _BI++;
            }
         }
      }
      else
      {  //because the previous beam was flat the links to remove are
         //below the last insert position
         kbLink* link;
         link = _low->GetBinHighest(true);
         if( link  )//is there something to remove
         {
            link->SetRecordNode( NULL );
 
            bool linkf = false;
            _BI.tohead();
            while (!_BI.hitroot())
            {
               kbRecord* record = _BI.item();
               if (record->GetLink() == link) 
                  linkf = true;
               _BI++;
            }
 
            if ( !linkf )
               _BI.tohead();
 
 
            if ( scantype == LINKHOLES )
            {
               _BI.tohead();
               while (!_BI.hitroot())
               {
                  kbRecord* record=_BI.item();
                  //records containing links towards the new low node
                  //are links to be removed
                  if ((record->GetLink()->GetEndNode() == _low) ||
                      (record->GetLink()->GetBeginNode() == _low)
                     )
                  {
                     holes = ProcessHoles(false,_I) || holes;
                  }
                  _BI++;
               }
            }
 
            //_BI.tonode( link->GetRecordNode() );
            //delete _BI.item();
            //_BI.remove();
 
            //on record back bring us to the last inserted record
            //or if nothing was inserted the record before the last deleted record
            //if there was no record before the last deleted record this means
            //we where at the beginning of the beam, so at root
 
            //_BI << (lastinserted+1);
            //_BI--;
            //if (_BI.hitroot())  //only possible when at the begin of the beam
 
            //found=false;
 
            _BI.tohead();
            while (!_BI.hitroot())
            {
               kbRecord* record=_BI.item();
               //records containing links towards the new low node
               //are links to be removed
               if ((record->GetLink()->GetEndNode() == _low) ||
                   (record->GetLink()->GetBeginNode() == _low)
                  )
               {
                  if ( link != record->GetLink() )
                  {
                     break;
                  }
                  if ( link->GetRecordNode() != _BI.node() )
                  {
                     delete _BI.item();
                     _BI.remove();
                  }
                  else
                  {
                     delete _BI.item();
                     _BI.remove();
                  }
                  found=true;
               }
               else if (found) //only once in here
                  break;
               else if (record->Ysp() < _low->GetY())
                     //if flatlinks are not in the beam nothing will be found
                     //this will bring us to the right insertion point
                     break;
               else
                  _BI++;
            }
 
         }
 
         else
         {
 
            //on record back bring us to the last inserted record
            //or if nothing was inserted the record before the last deleted record
            //if there was no record before the last deleted record this means
            //we where at the beginning of the beam, so at root
 
            //_BI << (lastinserted+ 1);
            //_BI--;
            //if (_BI.hitroot())  //only possible when at the begin of the beam
               _BI.tohead();
            while (!_BI.hitroot())
            {
                  kbRecord* record=_BI.item();
                  if (record->Ysp() < _low->GetY())
                     break;
                  _BI++;
            }
 
         }
      }
      break;
 
      case NODELINK:
      case LINKLINK:
      {
         if (_type == NORMAL)
         {
            Calc_Ysp();
            if (scantype==LINKLINK)
               foundnew = Process_LinkToLink_Crossings() !=0 || foundnew;
            else
               SortTheBeam( false );
         }
         //else beam is already sorted because the added/removed flat links
         //do not change the ysp of links already there, new non flat links
         //are inserted in order, as result the beam stays sorted
 
         if (_low->GetBinHighest(true)) //is there something to remove
         {
            _BI.tohead();
            while (!_BI.hitroot())
            {
               kbRecord* record=_BI.item();
               //records containing links towards the new low node
               //are links to be removed
               if ((record->GetLink()->GetEndNode() == _low) ||
                   (record->GetLink()->GetBeginNode() == _low)
                  )
               {
                  kbLine* line=record->GetLine();
                  if (scantype==NODELINK)
                   foundnew = Process_PointToLink_Crossings() !=0 || foundnew;
                  line->ProcessCrossings(_I);
                  delete _BI.item();
                  _BI.remove();
                  found=true;
               }
               //because the beam is sorted on ysp, stop when nothing can be there to remove
               //and the right insertion point for new links has been found
               else if ((record->Ysp() < _low->GetY()))
                  break;
               else
                  _BI++;
            }
         }
         else
         {
            _BI.tohead();
            while (!_BI.hitroot())
            {
                  kbRecord* record=_BI.item();
                  //because the beam is sorted on ysp, stop when
                  //the right insertion point for new links has been found
                  if ((record->Ysp() < _low->GetY()))
                     break;
                  _BI++;
            }
         }
      }
      break;
 
      default: 
         break; 
   }
 
 return foundnew;
}
*/

void ScanBeam::SortTheBeam( bool backangle )
{
    if ( backangle )
        _BI.mergesort( recordsorter_ysp_angle_back );
    else
        _BI.mergesort( recordsorter_ysp_angle );
}

void ScanBeam::Calc_Ysp()
{
    _BI.tohead();
    while ( !_BI.hitroot() )
    {
        kbRecord * record = _BI.item();
//  kbLink* link=_BI.item()->GetLink();
        record->Calc_Ysp( _low );
        _BI++;
    }
}

// this function will set for all the records which contain a link with the
// corresponding graphnumber the inc flag.
// The inc flag's function is to see in a beam if we go deeper in the graph or not
void ScanBeam::Generate_INOUT( int graphnumber )
{
    DIRECTION first_dir = GO_LEFT;
    int diepte          = 0;

    DL_Iter<kbRecord*> _BBI = DL_Iter<kbRecord*>();
    _BBI.Attach( this );
    for( _BBI.tohead(); !_BBI.hitroot(); _BBI++ )
    {
        kbRecord* record = _BBI.item();
        // recalculate _inc again
        if ( record->GetLink()->GetGraphNum() == graphnumber )
        { //found a link that belongs to the graph
            if ( diepte == 0 )
            { // first link found or at depth zero again
                // the direction is important since this is used to find out
                // if we go further in or out for coming links
                first_dir = record->Direction();
                record->GetLink()->SetInc( true );
                diepte = 1;
            }
            else
            { // according to depth=1 links set depth
                // verhoog of verlaag diepte
                if ( record->Direction() == first_dir )
                {
                    diepte++;
                    record->GetLink()->SetInc( true );
                }
                else
                {
                    diepte--;
                    record->GetLink()->SetInc( false );
                }
            }
        }
        if ( record == _BI.item() ) break; //not need to do the rest, will come in a later beam
    }
    _BBI.Detach();
}


// function ProcessHoles
//
// this function will search the closest link to a hole
// step one, search for a link that is marked (this is a hole)
// step two, the closest link is the previous link in
//     the beam, but only in the beam that contains the highest node
//     from the marked link.
//     why ? : if the marked link has for the begin and end node different
//     x,y values, see below as link C
//                                 B
//               A            +---------+
//          +----------+
//                     ___--+
//                  ___---
//                +---    C
//
//     when we at first detect link C we would link it to link A, should work he
//     but; we always link a hole at its topleft node, so the highest node
//     and then we can't link to A but we should link to B
//     so when we found the link, we will look if the node that will come
//     in a later beam will be higher than the current, if so we will wait
//     till that node comes around otherwise we will link this node to the
//     closest link (prev in beam)
bool ScanBeam::ProcessHoles( bool atinsert, TDLI<kbLink>* _LI )
{
    // The scanbeam must already be sorted at this moment
    kbNode * topnode;
    bool foundholes = false;

    kbRecord* record = _BI.item();
    kbLink* link = record->GetLink();
    kbNode* _low = _LI->item()->GetBeginNode();

    if ( _GC->GetLinkHoles() )
    {
        if( _GC->GetAllowNonTopHoleLinking() )
        {
            _BI++;
            if ( !_BI.hitroot() && _BI.item()->GetLink()->IsTopHole() )
            {      

                kbLink* linkToThis = _BI.item()->GetLink();
                //calculate linkToThis its Y at X of topnode.
                kbLine line( _GC );
                line.Set( linkToThis );
                B_INT Y;
                kbNode * leftnode; //left node of clossest link
                //link right now
                if ( linkToThis->GetEndNode()->GetX() == _low->GetX() )
                    Y = linkToThis->GetEndNode()->GetY();
                else if ( linkToThis->GetBeginNode()->GetX() == _low->GetX() )
                    Y = linkToThis->GetBeginNode()->GetY();
                else
                    Y =  line.Calculate_Y( _low->GetX() );

                if ( linkToThis->GetBeginNode()->GetX() < linkToThis->GetEndNode()->GetX() ) 
                    leftnode = linkToThis->GetBeginNode();
                else
                    leftnode = linkToThis->GetEndNode();
                kbNode *topnode = new kbNode( _low->GetX(), Y, _GC );
                kbLink *link_A = new kbLink( 0, topnode, leftnode, _GC );
                // the orginal linkToThis
                linkToThis->Replace( leftnode, topnode );
                _LI->insbegin( link_A );
                //reset mark to flag that this hole has been processed
                linkToThis->SetTopHole( false );

                kbLink *link_B = new kbLink( 0, _low, topnode, _GC );
                kbLink *link_BB = new kbLink( 0, topnode, _low, _GC );       
                _LI->insbegin( link_B );
                _LI->insbegin( link_BB );
                //mark those two segments as hole linking segments
                link_B->SetHoleLink( true );
                link_BB->SetHoleLink( true );

                //is where we come from/link to a hole
                bool closest_is_hole = linkToThis->GetHole();

                // if the polygon linked to, is a hole, this hole here
                // just gets bigger, so we take over the links its hole marking.
                link_A->SetHole( closest_is_hole );
                link_B->SetHole( closest_is_hole );
                link_BB->SetHole( closest_is_hole );

                // we have only one operation at the time, taking
                // over the operation flags is enough, since the linking segments will
                // be part of that output for any operation done.
                link_A->TakeOverOperationFlags( linkToThis );
                link_B->TakeOverOperationFlags( linkToThis );
                link_BB->TakeOverOperationFlags( linkToThis );

                foundholes = true;

                SortTheBeam( atinsert );
            }
            _BI--;
        }

        if ( !record->GetLine()->GetLink()->CrossListEmpty() )
        {
            SortTheBeam( atinsert );

            // link the holes in the graph to a link above.
            // a the link where the linecrosslist is not empty, means that
            // there are links which refer to this link (must be linked to this link)
            // make new nodes and links and set them, re-use the old link, so the links
            // that still stand in the linecrosslist will not be lost.
            // There is a hole that must be linked to this link !

            kbLink* linkToThis = link;

            TDLI<kbNode> I( record->GetLine()->GetLink()->GetCrossList() );
            I.tohead();
            while( !I.hitroot() )
            {
                topnode = I.item();
                I.remove();

                //calculate linkToThis its Y at X of topnode.
                kbLine line( _GC );
                line.Set( linkToThis );

                kbNode * leftnode; //left node of clossest link
                B_INT Y;
                //check if flatlink ( i think is always linkBB from a topnode at same X coordinate.
                //but lets accept all flatlinks 
                if ( linkToThis->GetEndNode()->GetX() == linkToThis->GetBeginNode()->GetX() )
                {
                    //we take the lowest of the flatlink nodes, which is right for
                    // a second hole at same X
                    if ( linkToThis->GetEndNode()->GetY() >= linkToThis->GetBeginNode()->GetY() )
                    {
                        Y = linkToThis->GetBeginNode()->GetY(); 
                        leftnode = linkToThis->GetBeginNode();
                    }
                    else
                    {
                        Y = linkToThis->GetEndNode()->GetY();
                        leftnode = linkToThis->GetEndNode();
                    }
                }
                else
                {   
                    if ( linkToThis->GetEndNode()->GetX() == topnode->GetX() )
                        Y = linkToThis->GetEndNode()->GetY();
                    else if ( linkToThis->GetBeginNode()->GetX() == topnode->GetX() )
                        Y = linkToThis->GetBeginNode()->GetY();
                    else
                        Y =  line.Calculate_Y( topnode->GetX() );

                    if ( linkToThis->GetBeginNode()->GetX() < linkToThis->GetEndNode()->GetX() ) 
                        leftnode = linkToThis->GetBeginNode();
                    else
                        leftnode = linkToThis->GetEndNode();
                }
                // Now we'll create new nodes and new links to make the link between
                // the graphs.

                //holes are always linked in a non hole or hole
                //for a non hole this link will be to the right
                //because non holes are right around
                //for holes this will be to the right also,
                //because they are left around but the link is always on the
                //bottom of the hole

                //    linkA                      linkToThis
                //   o-------->--------NodeA------->------------o
                //  leftnode          |  |
                //                    |  |
                //             linkB  v  ^ linkBB
                //                    |  |
                //                    |  |
                //   outgoing*        |  |          incoming*
                //   o------<---------topnode--------<----------o
                //
                // all holes are oriented left around


                kbNode *node_A = new kbNode( topnode->GetX(), Y, _GC );
                kbLink *link_A = new kbLink( 0, leftnode, node_A, _GC );
                kbLink *link_B = new kbLink( 0, node_A, topnode, _GC );
                kbLink *link_BB = new kbLink( 0, topnode, node_A, _GC );
                // the orginal linkToThis
                linkToThis->Replace( leftnode, node_A );
                _LI->insbegin( link_A );
                _LI->insbegin( link_B );
                _LI->insbegin( link_BB );

                //mark those two segments as hole linking segments
                link_B->SetHoleLink( true );
                link_BB->SetHoleLink( true );

                //is where we come from/link to a hole
                bool closest_is_hole = linkToThis->GetHole();

                // if the polygon linked to, is a hole, this hole here
                // just gets bigger, so we take over the links its hole marking.
                link_A->SetHole( closest_is_hole );
                link_B->SetHole( closest_is_hole );
                link_BB->SetHole( closest_is_hole );

                // we have only one operation at the time, taking
                // over the operation flags is enough, since the linking segments will
                // be part of that output for any operation done.
                link_A->TakeOverOperationFlags( linkToThis );
                link_B->TakeOverOperationFlags( linkToThis );
                link_BB->TakeOverOperationFlags( linkToThis );

                // check next top node is at same X
                if ( !I.hitroot() )
                {
                    kbNode *newtopnode = I.item();
                    if ( topnode->GetX() == newtopnode->GetX() )
                        linkToThis = link_BB;
                    else
                        linkToThis = link;
                }
            }
        }
    }

    if ( link->IsTopHole() )
    {
        SortTheBeam( atinsert );
        writebeam();
    }

    if ( link->IsTopHole() && !_BI.athead() )
    {
        // now we check if this hole should now be linked, or later
        // we always link on the node with the maximum y value, Why ? because i like it !
        // to link we put the node of the hole into the crosslist of the closest link !

        assert( record->Direction() == GO_LEFT );
        // he goes to the left
        if ( atinsert )
        {
            if ( link->GetBeginNode()->GetY() <= link->GetEndNode()->GetY() )
            {
                topnode = link->GetEndNode();
                //the previous link in the scanbeam == the closest link to the hole in vertical
                //direction PUT this node into this link
                _BI--;
                int graphnumber = _BI.item()->GetLine()->GetLink()->GetGraphNum();
                _BI.item()->GetLine()->GetLink()->AddCrossing( topnode );
                _BI++;
                //reset tophole flag, hole has been processed
                link->SetTopHole( false );
                foundholes = true;
            }
        }
        else  //remove stage of links from te beam
        {
            //the tophole link was NOT linked at the insert stage, so it most be linked now
            topnode = _BI.item()->GetLink()->GetBeginNode();
            //the previous link in the scanbeam == the closest link to the hole in vertical
            //direction PUT this node into this link
            _BI--;
            int graphnumber = _BI.item()->GetLine()->GetLink()->GetGraphNum();
            _BI.item()->GetLine()->GetLink()->AddCrossing( topnode );
            _BI++;
            //reset mark to flag that this hole has been processed
            link->SetTopHole( false );
            foundholes = true;
        }
    }
    return foundholes;
}

//sort the records on Ysp if eqaul, sort on tangent at ysp
int recordsorter_ysp_angle( kbRecord* rec1, kbRecord* rec2 )
{
    if ( rec1->Ysp() > rec2->Ysp() )
        return( 1 );
    if ( rec1->Ysp() < rec2->Ysp() )
        return( -1 );
    //it seems they are equal
    B_INT rightY1;
    if ( rec1->Direction() == GO_LEFT )
        rightY1 = rec1->GetLink()->GetBeginNode()->GetY();
    else
        rightY1 = rec1->GetLink()->GetEndNode()->GetY();
    B_INT rightY2;
    if ( rec2->Direction() == GO_LEFT )
        rightY2 = rec2->GetLink()->GetBeginNode()->GetY();
    else
        rightY2 = rec2->GetLink()->GetEndNode()->GetY();

    if ( rightY1 > rightY2 )
        return( 1 );
    if ( rightY1 < rightY2 )
        return( -1 );
    return( 0 );
}

//sort the records on Ysp if eqaul, sort on tangent at ysp
int recordsorter_ysp_angle_back( kbRecord* rec1, kbRecord* rec2 )
{
    if ( rec1->Ysp() > rec2->Ysp() )
        return( 1 );
    if ( rec1->Ysp() < rec2->Ysp() )
        return( -1 );
    //it seems they are equal
    B_INT leftY1;
    if ( rec1->Direction() == GO_RIGHT )
        leftY1 = rec1->GetLink()->GetBeginNode()->GetY();
    else
        leftY1 = rec1->GetLink()->GetEndNode()->GetY();
    B_INT leftY2;
    if ( rec2->Direction() == GO_RIGHT )
        leftY2 = rec2->GetLink()->GetBeginNode()->GetY();
    else
        leftY2 = rec2->GetLink()->GetEndNode()->GetY();

    if ( leftY1 > leftY2 )
        return( 1 );
    if ( leftY1 < leftY2 )
        return( -1 );
    return( 0 );
}

// swap functie for cocktailsort ==> each swap means an intersection of links
bool swap_crossing_normal( kbRecord *a, kbRecord *b )
{
    if ( !a->Equal( b ) ) // records NOT parallel
    {
        a->GetLine()->Intersect_simple( b->GetLine() );
        return true;
    }
    return false;
}

int ScanBeam::Process_LinkToLink_Crossings()
{
    // sort on y value of next intersection; and find the intersections
    return _BI.cocktailsort( recordsorter_ysp_angle_back, swap_crossing_normal );
}

//catch node to link crossings
// must be sorted on ysp
int ScanBeam::Process_PointToLink_Crossings()
{
    int merges = 0;
    kbRecord* record;

    if ( _BI.count() > 1 )
    {
        DL_Iter<kbRecord*> IL = DL_Iter<kbRecord*>( this );
        IL.toiter( &_BI );

        //from IL search back for close links
        IL--;
        while( !IL.hitroot() )
        {
            record = IL.item();

            if ( record->Ysp() > _low->GetY() + _GC->GetInternalMarge() )
                break;

            // the distance to the lo/hi node is smaller then the _GC->GetInternalMarge()
            if( ( record->GetLink()->GetBeginNode() != _low ) &&
                    ( record->GetLink()->GetEndNode()  != _low )
              )
            {  // the link is not towards the lohi node
                record->GetLine()->GetLink()->AddCrossing( _low );
                merges++;
            }
            IL--;
        }

        //from IL search forward for close links
        IL.toiter( &_BI );
        IL++;
        while( !IL.hitroot() )
        {
            record = IL.item();

            if ( record->Ysp() < _low->GetY() - _GC->GetInternalMarge() )
                break;

            // the distance to the lohi node is smaller then the booleng->Get_Marge()
            if( ( record->GetLink()->GetBeginNode() != _low ) &&
                    ( record->GetLink()->GetEndNode()  != _low )
              )
            {  // the link is not towards the low node
                record->GetLine()->GetLink()->AddCrossing( _low );
                merges++;
            }
            IL++;
        }

    }

    return merges;
}

int ScanBeam::Process_LinkToLink_Flat( kbLine* flatline )
{
    int crossfound = 0;
    kbRecord* record;
    DL_Iter<kbRecord*> _BBI = DL_Iter<kbRecord*>();
    _BBI.Attach( this );
    _BBI.toiter( &_BI );

    for( _BI.tohead(); !_BI.hitroot(); _BI++ )
    {
        record = _BI.item();

        if ( record->Ysp() < ( flatline->GetLink()->GetLowNode()->GetY() - _GC->GetInternalMarge() ) )
            break;//they are sorted so no other can be there

        if ( ( record->Ysp() > ( flatline->GetLink()->GetLowNode()->GetY() - _GC->GetInternalMarge() ) )
                &&
                ( record->Ysp() < ( flatline->GetLink()->GetHighNode()->GetY() + _GC->GetInternalMarge() ) )
           )
        { //it is in between the flat link region
            //create a new node at ysp and insert it in both the flatlink and the crossing link

            if (
                ( record->GetLink()->GetEndNode()  != flatline->GetLink()->GetHighNode() ) &&
                ( record->GetLink()->GetEndNode()  != flatline->GetLink()->GetLowNode() ) &&
                ( record->GetLink()->GetBeginNode() != flatline->GetLink()->GetHighNode() ) &&
                ( record->GetLink()->GetBeginNode() != flatline->GetLink()->GetLowNode() )
            )
            {
                kbNode * newnode = new kbNode( _low->GetX(), _BI.item()->Ysp(), _GC );
                flatline->GetLink()->AddCrossing( newnode );
                record->GetLine()->GetLink()->AddCrossing( newnode );
                crossfound++;
            }
        }
    }

    _BI.toiter( &_BBI );
    _BBI.Detach();
    return crossfound;
}

bool ScanBeam::checksort()
{
    // if empty then just insert
    if ( empty() )
        return true;

    // put new item left of the one that is bigger
    _BI.tohead();
    kbRecord* prev = _BI.item();
    _BI++;
    while( !_BI.hitroot() )
    {
        kbRecord * curr = _BI.item();
        if ( recordsorter_ysp_angle( prev, curr ) == -1 )
        {
            recordsorter_ysp_angle( prev, curr );
            return false;
        }
        prev = _BI.item();
        _BI++;
    }
    return true;
}

bool ScanBeam::writebeam()
{
    if ( !_GC->GetDebug() )
        return true;

    FILE * file = _GC->GetLogFile();

    if ( file == NULL )
        return true;

    fprintf( file, "# beam %d \n", count() );
    fprintf( file, " low %I64d %I64d \n", _low->GetX() , _low->GetY() );
    fprintf( file, " type %d \n", _type );
    if ( !_BI.hitroot() )
    {
        kbRecord* itercur = _BI.item();
        fprintf( file, " iterysp %I64d \n", itercur->Ysp() );
    }
    else
        fprintf( file, " iterysp at end \n" );

    if ( empty() )
    {
        fprintf( file, "             empty \n" );
        return true;
    }

    DL_Iter<kbRecord*> _BI2( this );

    // put new item left of the one that is bigger
    _BI2.tohead();
    while( !_BI2.hitroot() )
    {
        kbRecord * cur = _BI2.item();

        fprintf( file, " ysp %I64d \n", cur->Ysp() );

        kbLink* curl = cur->GetLink();

        fprintf( file, "             linkbegin %I64d %I64d \n", curl->GetBeginNode()->GetX(), curl->GetBeginNode()->GetY() );
        fprintf( file, "             linkend %I64d %I64d \n", curl->GetEndNode()->GetX(), curl->GetEndNode()->GetY() );

        if ( curl->GetEndNode()->GetX() == -2459565876494606883 )
            fprintf( file, "             linkend %I64d %I64d \n", curl->GetEndNode()->GetX(), curl->GetEndNode()->GetY() );

        _BI2++;
    }

    return true;
}
