#include <windows.h>
#include "katanox.h"
#include "anoxcam.h"
#include "util.h"


//--------------------
// CAMERA::CAMERA()
//
// default constructor.
//--------------------

CAMERA::CAMERA()
{
    name[0] = 0x00;
    NodeHead = NULL;
    NodeTail = NULL;
    NodeCurrent = NULL;
    iCamFocus = -1;

    iThisNode = iTotalNodes = 0;
}


//--------------------
// CAMERA::~CAMERA()
//
// The default destructor which also deletes all the nodes in the camera's
// list before poofing out of existence.
//--------------------

CAMERA::~CAMERA()
{
    CAMNODE *np, *lastnp;

    np = NodeHead;

    while( np )
    {
        lastnp = np;
        np = np->next;
        delete lastnp;
    }
}


//--------------------
// CAMERA::AppendNode
//
// Appends a node to the end of the camera's nodelist.
//--------------------

void CAMERA::AppendNode( CAMNODE *pNode )
{
    NodeCurrent = NULL;
    InsertNode( pNode );
}


//--------------------
// CAMERA::InsertNode
//
// Inserts node BEFORE current node and makes it the current node.  If
// the current node is NULL then it appends the node to the end of the
// nodelist.
//--------------------

void CAMERA::InsertNode( CAMNODE *pNode )
{
    if( pNode == NULL )
    {
        MakeScriptError( "ERROR: Cannot insert a NULL node!" );
    }
    else
    {

        if( NodeHead == NULL )
        {
            NodeHead = NodeTail = NodeCurrent = pNode;
            iThisNode = iTotalNodes = 1;
            pNode->prev = NULL;
            pNode->next = NULL;
        }
        else
        {
            if( NodeCurrent == NULL ) // then append node as tail
            {
                pNode->prev = NodeTail;
                pNode->next = NULL;
                NodeTail->next = pNode;
                NodeTail = pNode;
            }
            else // do a normal insert
            {
                pNode->prev = NodeCurrent->prev;
                pNode->next = NodeCurrent;
                NodeCurrent->prev = pNode;

                if( NodeCurrent == NodeHead )
                {
                    NodeHead = pNode;
                }
                else
                {
                    pNode->prev->next = pNode;
                }
            }

            //---------------------------------
            // no changes to iThisNode because
            // it is the same place in line
            //---------------------------------
        
            ++iTotalNodes;
        } // end nodehead != NULL

        NodeCurrent = pNode;
    } // end node != NULL
}


//--------------------
// CAMERA::DeleteNode
//
// Deletes the current node (frees its memory), and sets the current node to 
// the next node.
//--------------------

void CAMERA::DeleteNode()
{
    CAMNODE *p;

    if( NodeCurrent )
    {
        if( NodeCurrent == NodeHead )
        {
            NodeHead = NodeCurrent->next;
        }
        else // not head, so safe to adjust previous node
        {
            NodeCurrent->prev->next = NodeCurrent->next;
        }

        if( NodeCurrent == NodeTail ) 
        {
            NodeTail = NodeCurrent->prev;
        }
        else // not tail, so safe to adjust next node
        {
            NodeCurrent->next->prev = NodeCurrent->prev;
        }

        p = NodeCurrent->next;
        delete( NodeCurrent );
        NodeCurrent = p;

        --iTotalNodes;
    }
}


//--------------------
// CAMERA::PrevNode
//
// Sets the current node to its previous node, unless it is the first
// node in the list.
//--------------------

void CAMERA::PrevNode()
{
    if( NodeCurrent == NULL )
    {
        NodeCurrent = NodeTail;
    }
    else if( NodeCurrent->prev )
    {
        NodeCurrent = NodeCurrent->prev;
    }
}


//--------------------
// CAMERA::NextNode
//
// Sets the current node to its next node, unless it is past the last node
// in the list (is NULL).
//--------------------

void CAMERA::NextNode()
{
    if( NodeCurrent )
    {
        NodeCurrent = NodeCurrent->next;
    }
}


//--------------------
// CAMERA::FirstNode
//
// Sets the current node to the first in the list.
//--------------------

void CAMERA::FirstNode()
{
    NodeCurrent = NodeHead;
}


//--------------------
// CAMERA::LastNode
//
// Sets the current node to the first in the list.
//--------------------

void CAMERA::LastNode()
{
    NodeCurrent = NodeTail;
}


//--------------------
// CAMERA::Init
//
// Inits data for playback from current node.
//--------------------

void CAMERA::Init()
{
    if( NodeCurrent )
    {
        NodeDataCopy = *NodeCurrent;
    }
}


//--------------------
// CAMERA::Init
//
// Does a complete init of all nodes in preparation for playback iterations.
//--------------------

void CAMERA::Init( const VEC3& initpos, const VEC3& initvel )
{
    CAMNODE *np;

    np = NodeHead;

    if( np )
    {
        np->Init( initpos, initvel );
        np = np->next;

        while( np )
        {
            np->Init();
            np = np->next;
        }
    }

    NodeCurrent = NodeHead;
    Init();
    iThisNode = 1;
}


//--------------------
// CAMERA::Iterate
//
// Does chaining and all that good stuff.  Takes the leftover time and shoves
// it into the next node.
//
// Returns one of CAMSTATE_*
//
// Returns CAMSTATE_IDLE when reached the end of all nodes.
// Returns CAMSTATE_ENDSCRIPT when reached END of a node with NODEFLAG_ENDSCRIPT
// Returns CAMSTATE_MAKECURRENT when reached START of a node with
//     NODEFLAG_MAKECURRENT.  It is the responsibility of the scriptor at the
//     beginning of script play to find the camera with the first node having
//     a MAKECURRENT flag, otherwise default to the first camera with nodes.
// Returns CAMSTATE_PLAYING if none of the above conditions are met.
//--------------------


int CAMERA::Iterate( float frametime )
{
    int iRetVal;
    float fTimeOver;

    if( NodeCurrent )
    {
        fTimeOver = NodeDataCopy.Iterate(frametime);
        if( fTimeOver > 0.0f )
        {
            iRetVal = CAMSTATE_NORMAL;
            pos = NodeDataCopy.pos;
            vel = NodeDataCopy.vel;
        }
        else
        {
            // keep going until we have a positive time value
            // or script ends.

            while( ( fTimeOver <= 0.0 ) && ( NodeCurrent ) )
            {
                ++iThisNode;

                if( NodeDataCopy.ulFlags & NODEFLAG_ENDSCRIPT )
                {
                    iRetVal = CAMSTATE_ENDSCRIPT;
                    pos = NodeDataCopy.pos;
                    vel = VEC3();
                    NodeCurrent = NULL;
                }
                else
                {
                    NodeCurrent = NodeCurrent->next;
                    if( NodeCurrent )
                    {
                        NodeDataCopy = *NodeCurrent;
                        NodeDataCopy.FastInit();
                        fTimeOver = NodeDataCopy.Iterate( -fTimeOver );
                        pos = NodeDataCopy.pos;
                        vel = NodeDataCopy.vel;

                        if( NodeDataCopy.ulFlags & NODEFLAG_MAKECURRENT )
                        {
                            iRetVal = CAMSTATE_MAKECURRENT;
                        }
                        else
                        {
                            iRetVal = CAMSTATE_NORMAL;
                        }
                    }
                    else // that was the last node
                    {
                        iRetVal = CAMSTATE_IDLE;
                    }
                }
            }
        }
    }
    else
    {
        iRetVal = CAMSTATE_IDLE;
    }

    return iRetVal;
}


