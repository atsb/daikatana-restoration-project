//#include "p_user.h"
//#include "common.h"

//extern common_export_t *com;


//void *DoMalloc(size_t size, mem_type type) {
//    //get some memory.
//    void *memory;
//
//	MEM_TAG tag = (type == MEM_PERMANENT) ? MEM_TAG_MISC : MEM_TAG_COM_MISC;
//
//	// see if mem manager is installed yet
//	if (com)
//	{
//		memory = com->X_Malloc(size,tag);
//	}
//	else
//	{
//		memory = malloc(size);
//	}
//
////    //compute the type of memory we will actually ask for.
////    mem_type use_type;
////
////	use_type = type;
////
////    if (type == HOOK_MALLOC) 
////		use_type = MEM_MALLOC;
////	else if (type == HOOK_HUNK) 
////		use_type = MEM_HUNK;
////	else if (type == HOOK_PERMANENT) 
////		use_type = MEM_PERMANENT;
////
////    if (memory_com == NULL) {
////        memory = malloc(size);
////    }
////    else {
////        memory = memory_com->com_malloc(size, use_type);
////    }
////
////    //check the type.
////    if (type == HOOK_MALLOC || type == HOOK_HUNK || type == HOOK_PERMANENT) {
////        //clear out the memory.
////        memset(memory, 0, size);
////    }
//
//    return memory;
//}
//
//void *operator new(size_t size, mem_type type) {
//    return DoMalloc(size, type);
//}
//
////void *operator new(size_t size) 
////{
////    return DoMalloc(size, HOOK_MALLOC);
////}
//
//void *operator new(size_t size, mem_type type, size_t real_size) {
//    return DoMalloc(real_size, type);
//}
//
//#if !_MSC_VER
////unix - new[]
//void *operator new[](size_t size, mem_type type, size_t real_size) {
//    return DoMalloc(real_size, type);
//}
////unix - new[]
//void *operator new[](size_t size, mem_type type) {
//    return DoMalloc(size, type);
//}
////unix - delete[]
//void operator delete[](void *ptr) {
////    if (memory_com == NULL || memory_com->com_free == NULL) 
////		free(ptr);
////	else
////		memory_com->com_free(ptr);
//	if (com)
//	{
//		com->X_Free(ptr);
//	}
//	else
//	{
//		free(ptr);
//	}
//}
//#endif
//
//void operator delete(void *ptr) {
////    if (memory_com == NULL || memory_com->com_free == NULL) 
////		free(ptr);
////	else
////		memory_com->com_free(ptr);
//	if (com)
//	{
//		com->X_Free(ptr);
//	}
//	else
//	{
//		free(ptr);
//	}
//}


