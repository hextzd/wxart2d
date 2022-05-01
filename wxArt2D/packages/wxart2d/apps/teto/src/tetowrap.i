#include "docv.h"
#include "wx/docview/doccom.h"
#include "wx/artbase/drawer2d.h"
#include "wx/editor/cancom.h"
#include "wx/editor/candocproc.h"

#define_string  a2dTETO_VERSION 

class %delete TetoLuaProcCommandProcessor : public a2dLuaCentralCommandProcessor
{

    TetoLuaProcCommandProcessor( long flags = a2dDEFAULT_DOCUMENT_FLAGS, bool initialize = true, int maxCommands = -1 )
    
    bool ShowDlg( const a2dCommandId* comID, bool modal = false, bool onTop = true )
};    

%function TetoLuaProcCommandProcessor* a2dGetGedi()


