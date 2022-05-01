/*! \file kbool/samples/boolonly/boolonly.h
    \author Klaas Holwerda
 
    Copyright: 2001-2004 (C) Klaas Holwerda
 
    Licence: see kboollicense.txt 
 
    RCS-ID: $Id: boolonly.h,v 1.5 2009/02/06 21:33:03 titato Exp $
*/

#ifndef BOOLONLY_H
#define BOOLONLY_H

#include "kbool/booleng.h"
#include "kbool/_lnk_itr.h"

class kbPoint
{
public:

    kbPoint();
    kbPoint( double const , double const );

    double m_x;
    double m_y;

};

#endif // BOOLONLY_H
