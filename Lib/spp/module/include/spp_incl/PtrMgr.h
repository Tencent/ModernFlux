/*
 * =====================================================================================
 *
 *       Filename:  PtrMgr.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/28/2010 05:05:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#ifndef __PTRMGR_H__
#define __PTRMGR_H__
#include <set>
#include "CommDef.h"

BEGIN_ASYNCFRAME_NS

typedef enum 
{
    PtrType_New = 0,
    PtrType_Malloc,
    PtrType_NewArray,
}PtrType;

/**
 * 堆上分配的内存地址管理，避免在进程退出时没有释放已分配的内存
 *
 */
template< typename T, PtrType type>
class CPtrMgr
{
    public:
        typedef typename std::set<T*>    PtrSet;
        typedef typename PtrSet::iterator PtrSetIter;
    public:
        ~CPtrMgr()
        {
            PtrSetIter it = _ptr_set.begin();
            for(; it != _ptr_set.end(); it++ )
            {
                Release( *it );
            }

            _ptr_set.clear();
        }

        void Register( T* ptr )
        {
            /*
            std::set<T*>::iterator it = _ptr_set.find( ptr );
            if( it != _ptr_set.end() )
            {
                return ;
            }
            */

            _ptr_set.insert(ptr);
        }

        void Recycle( T* ptr , bool need_release = true)
        {
            if( need_release )
            {
                Release( ptr );
            }
            _ptr_set.erase( ptr );
        }

    private:
        void Release( T* ptr )
        {
            if( type == PtrType_New )
            {
                delete ptr;
            }
            else if( type == PtrType_Malloc )
            {
                free(ptr);
            }
            else if( type == PtrType_NewArray )
            {
                delete [] ptr;
            }

        }

    private:
        PtrSet _ptr_set;

};

END_ASYNCFRAME_NS

#endif
