/*! \file canvas/src/hittest.cpp
    \brief general hittest functions
    \author Klaas Holwerda, Michael Sögtrop

    Copyright: 2001-2004 (C) Klaas Holwerda, Michael Sögtrop

    Licence: wxWidgets licence

    RCS-ID: $Id: hittest.cpp,v 1.15 2007/07/03 22:09:47 titato Exp $
*/

#include "a2dprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <float.h>
#include <wx/artbase/drawer2d.h>
#include <wx/canvas/hittest.h>

a2dHit HitTestRectangle( double xtest, double ytest, double xmin, double ymin, double xmax, double ymax, double margin )
{
    if ( xmin > xmax )
    {
        double x = xmax;
        xmax = xmin;
        xmin = x;
    }
    if ( ymin > ymax )
    {
        double y = ymax;
        ymax = ymin;
        ymin = y;
    }
    wxASSERT( margin >= 0 );

    static a2dHit hittypes [16] =
    {
        // 0000 = inside
        a2dHit( a2dHit::hit_fill,   a2dHit::stroke1_none, a2dHit::stroke2_none,     0, 0 ),
        // 0001 = left edge
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_edgevert, 0, 0 ),
        // 0010 = right edge
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_edgevert, 1, 0 ),
        // 0011 = degenerate (margin>width, left + right edge => use right edge)
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_edgevert, 1, 0 ),

        // 0100 = bottom edge
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_edgehor,  2, 0 ),
        // 0101 = bottom left vertex
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_vertex,   0, 0 ),
        // 0110 = bottom right vertex
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_vertex,   1, 0 ),
        // 0111 = degenerate bottom right+left vertex => use bootom right vertex
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_vertex,   1, 0 ),

        // 1000 = top edge
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_edgehor,  3, 0 ),
        // 1001 = top left vertex
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_vertex,   2, 0 ),
        // 1010 = top right vertex
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_vertex,   3, 0 ),
        // 1011 = degenerate top right+left vertex => use top right vertex
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_vertex,   3, 0 ),

        // 1100 = degenerate top+bottom edge => use bottom edge
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_edgehor,  2, 0 ),
        // 1101 = degenerate top+bottom left vertex => us bottom left vertex
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_vertex,   0, 0 ),
        // 1110 = degenerate top+bottom right vertex => us bottom right vertex
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_vertex,   1, 0 ),
        // 1111 = degenerate top+bottom degenerate left+right vertex => us bottom right vertex
        a2dHit( a2dHit::hit_stroke, a2dHit::stroke1_none, a2dHit::stroke2_vertex,   1, 0 )
    };

    a2dHit result;

    // check if inside bounding box with positive margin
    if (
        xtest < xmax + margin &&
        ytest < ymax + margin &&
        xtest > xmin - margin &&
        ytest > ymin - margin
    )
    {
        // check if inside bounding box with negative margin
        int hittype = 0;
        if ( xtest < xmin + margin ) hittype += 1;
        if ( xtest > xmax - margin ) hittype += 2;
        if ( ytest < ymin + margin ) hittype += 4;
        if ( ytest > ymax - margin ) hittype += 8;

        result = hittypes[hittype];
        if( result.m_hit == a2dHit::hit_stroke )
        {
            // need inside/outside flags
            if (
                xtest < xmax &&
                ytest < ymax &&
                xtest > xmin &&
                ytest > ymin
            )
                result.m_stroke1 = a2dHit::stroke1_inside;
            else
                result.m_stroke1 = a2dHit::stroke1_outside;
        }
    }

    return result;
}
