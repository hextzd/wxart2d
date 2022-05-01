/*! \file general/src/a2dmemmgr.cpp
    \author Klaas Holwerda / Leo Kadisoff

    Copyright: 2001-2005 (c) Klaas Holwerda

    Licence: wxWidgets Licence

    RCS-ID: $Id: a2dmemmgr.cpp,v 1.9 2007/07/30 19:34:11 titato Exp $
*/

#include "wxartbaseprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/general/a2dmemmgr.h"

#ifdef new
#undef new
#endif

a2dMemManager::~a2dMemManager()
{
    for( a2dFreeList* fl = m_allLists; fl != NULL; )
    {
        wxASSERT( m_count > 0 );
        for( void* space = fl->m_topOfList; space != NULL; )
        {
            wxASSERT( fl->m_count > 0 );
            void* itemForDestroy = space;
            space = *( ( void** )space );
            delete itemForDestroy;
            fl->m_count --;
        }
        a2dFreeList* forDestroy = fl;
        fl = fl->m_next;
        delete forDestroy;
        m_count--;
    }
}

void* a2dMemManager::Allocate( size_t bytes )
{
//  wxCriticalSectionLocker aGuard(m_guard);
    for( a2dFreeList* fl = m_allLists;
            fl != NULL;
            fl = fl->m_next )
    {
        if( fl->m_blockSize == bytes )
        {
            if( fl->m_topOfList != NULL )
            {
                void* space = fl->m_topOfList;
                fl->m_topOfList = *( ( void** )( fl->m_topOfList ) );
                fl->m_count--;
                wxASSERT( ( fl->m_topOfList == NULL && fl->m_count == 0 ) || ( fl->m_topOfList != NULL && fl->m_count != 0 ) );
                return space;
            }
            break;
        }
    }
    void* newpointer = ::operator new( bytes );
    return newpointer;
}

void a2dMemManager::Deallocate( void* space, size_t bytes )
{
//  wxCriticalSectionLocker aGuard(m_guard);

    a2dFreeList* fl;
    for( fl = m_allLists; fl != NULL; fl = fl->m_next )
    {
        if( fl->m_blockSize == bytes )
            break;
    }
    if( fl == NULL )
    {
        fl = new a2dFreeList( m_allLists, bytes );
        m_allLists = fl;
        m_count++;
    }

    *( ( void** )space ) = fl->m_topOfList;
    fl->m_topOfList = space;
    fl->m_count++;
}

#ifdef __WXDEBUG__
#if wxUSE_STACKWALKER
#include "wx/stackwalk.h"

wxString a2dGetStackTrace( size_t skip, bool withArgs, int maxCalls, int maxLines )
{
    wxString stackTrace;

    class StackDump : public wxStackWalker
    {
    public:
        StackDump( int maxCalls = 20, bool withArgs = false ) { m_curCall = 0; m_maxCalls = maxCalls; m_withArgs = withArgs; }

        const wxString& GetStackTrace() const { return m_stackTrace; }

    protected:
        virtual void OnStackFrame( const wxStackFrame& frame )
        {
            if( m_curCall >= m_maxCalls )
                return;
            m_curCall++;
            m_stackTrace << wxString::Format( _T( "\t[%02d] " ), frame.GetLevel() );

            wxString name = frame.GetName();
            if ( !name.empty() )
            {
                m_stackTrace << wxString::Format( _T( "%-40s" ), name.c_str() );
            }
            else
            {
                m_stackTrace << wxString::Format
                             (
                                 _T( "0x%08lx" ),
                                 ( unsigned long )frame.GetAddress()
                             );
            }

            if( m_withArgs )
            {
                wxString type, val;
                size_t n;
                for ( n = 0; frame.GetParam( n, &type, &name, &val ); n++ )
                {
                    if( !n )
                        m_stackTrace << wxT( "\t(" );
                    else
                        m_stackTrace << wxT( ", " );
                    if ( val.Find( '\n' ) != wxNOT_FOUND )
                        val = "..."; 
                    m_stackTrace << wxString::Format( "%s %s=%s", type.c_str(), name.c_str(), val.c_str() );
                }
                if( n )
                    m_stackTrace << wxT( " )" );
            }

            if ( frame.HasSourceLocation() )
            {
                m_stackTrace << _T( '\t' )
                             << frame.GetFileName()
                             << _T( ':' )
                             << frame.GetLine();
            }

            m_stackTrace << _T( '\n' );
        }

    private:
        wxString m_stackTrace;
        bool     m_withArgs;
        int      m_maxCalls;
        int      m_curCall;
    };

    StackDump dump( maxCalls, withArgs );
    dump.Walk( skip ); // don't show call itself
    stackTrace = dump.GetStackTrace();

    // don't show more than maxLines or we could get a dialog too tall to be
    // shown on screen: 20 should be ok everywhere as even with 15 pixel high
    // characters it is still only 300 pixels...
    const int count = stackTrace.Freq( wxT( '\n' ) );
    for ( int i = 0; i < count - maxLines; i++ )
        stackTrace = stackTrace.BeforeLast( wxT( '\n' ) );

    return stackTrace;
}
#endif // wxUSE_STACKWALKER
#endif // __WXDEBUG__
