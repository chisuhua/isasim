#define ReadReg item->ReadReg
#define ReadSReg item->ReadSReg
#define WriteSReg item->WriteSReg
#define ReadVReg item->ReadVReg
#define WriteVReg item->WriteVReg
#define	ReadBitmaskSReg item->ReadBitmaskSReg
#define	WriteBitmaskSReg item->WriteBitmaskSReg

#define	WriteLDS item->WriteLDS
#define	ReadLDS item->ReadLDS

#define	WriteMemory item->WriteMemory
#define	ReadMemory item->ReadMemory

#define	ReadBufferResource item->ReadBufferResource
#define	ReadMemPtr item->ReadMemPtr
#define	ReadVRegMemPtr item->ReadVRegMemPtr

#define	GetWarp item->GetWarp()
#define	GetBlock item->GetBlock()

inline void	ISAUnimplemented(ThreadItem*){
}

#include "executor/DataTypes.h"
