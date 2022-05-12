// FIXME
#define ReadReg item->getSreg

#define ReadSReg item->getSreg
#define WriteSReg item->setSreg
#define ReadVReg item->getVreg
#define WriteVReg item->setVreg
#define	ReadBitmaskSReg item->getBitmaskSreg
#define	WriteBitmaskSReg item->setBitmaskSreg

#define	WriteLDS item->setDmem
#define	ReadLDS item->getDmem

#define	WriteSMEM item->writeSMEM
#define	ReadSMEM item->readSMEM

#define	WriteDMEM item->writeDMEM
#define	ReadDMEM item->readDMEM

#define	WriteVMEM item->writeVMEM
#define	ReadVMEM item->readVMEM

#define	ReadBufferResource item->ReadBufferResource
#define	ReadMemPtr item->getSregMemPtr
#define	ReadVRegMemPtr item->getVregMemPtr

// #define	GetWarp item->GetWarp()
// #define	GetBlock item->GetBlock()

inline void	ISAUnimplemented(class WarpState*){
}

#include "common/string_utils.h"
#include "common/DataTypes.h"
#include "inc/ThreadBlock.h"
#include "inc/HwOp.h"

