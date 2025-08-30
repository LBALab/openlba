/*
 * P_ANIM.cpp - 3D Animation System
 * Converted from P_ANIM.ASM to C++
 */

#include "LIB_3D.H"
#include "../LIB_SYS/ADELINE.H"

extern WORD List_Anim_Point;

static INT Offset_Source = 0;
static INT Offset_Dest = 0;
static INT StartInfos = 0;

LONG MemoTimerRef = 0;
LONG CmptMemoTimerRef = 0;

WORD AnimStepX = 0;
WORD AnimStepY = 0;
WORD AnimStepZ = 0;

WORD AnimMasterRot = 0;
WORD AnimStepAlpha = 0;
WORD AnimStepBeta = 0;
WORD AnimStepGamma = 0;

static WORD CXSetInterAnim = 0;

extern LONG TimerRef;

void SaveTimer()
{
    if (CmptMemoTimerRef == 0) {
        MemoTimerRef = TimerRef;
    }
    CmptMemoTimerRef++;
}

void RestoreTimer()
{
    CmptMemoTimerRef--;
    if (CmptMemoTimerRef == 0) {
        TimerRef = MemoTimerRef;
    }
}

void SetAngleGroupe(INT numgroupe, INT palpha, INT pbeta, INT pgamma, INT ptrobj)
{
    UBYTE *esi = (UBYTE*)ptrobj;
    
    WORD ax = *(WORD*)esi;
    if (!(ax & INFO_ANIM)) return;
    
    esi += 14;
    
    WORD size_info = *(WORD*)esi;
    esi += size_info + 4;
    
    WORD nb_points = *(WORD*)(esi - 2);
    esi += nb_points * 6;
    
    WORD nb_groupes = *(WORD*)esi;
    
    if (numgroupe >= nb_groupes) return;
    
    esi += 2 + 10 + (numgroupe * 38);
    
    *(WORD*)esi = (WORD)palpha;
    *(WORD*)(esi + 2) = (WORD)pbeta;
    *(WORD*)(esi + 4) = (WORD)pgamma;
}

void GetAngleGroupe(INT numgroupe, INT ptrobj)
{
    UBYTE *esi = (UBYTE*)ptrobj;
    
    WORD ax = *(WORD*)esi;
    if (!(ax & INFO_ANIM)) return;
    
    esi += 14;
    
    WORD size_info = *(WORD*)esi;
    esi += size_info + 4;
    
    WORD nb_points = *(WORD*)(esi - 2);
    esi += nb_points * 6;
    
    WORD nb_groupes = *(WORD*)esi;
    
    if (numgroupe >= nb_groupes) return;
    
    esi += 2 + 10 + (numgroupe * 38);
    
    AnimStepAlpha = *(WORD*)esi;
    AnimStepBeta = *(WORD*)(esi + 2);
    AnimStepGamma = *(WORD*)(esi + 4);
}

LONG GetNbFramesAnim(INT ptranim)
{
    WORD *ecx = (WORD*)ptranim;
    return *ecx;
}

LONG GetBouclageAnim(INT ptranim)
{
    WORD *ecx = (WORD*)(ptranim + 4);
    return *ecx;
}

LONG SetAnimObjet(INT numframe, INT ptranim, INT ptrobj)
{
    UBYTE *esi = (UBYTE*)ptranim;
    UBYTE *edi = (UBYTE*)ptrobj;
    
    WORD nb_frames = *(WORD*)esi;
    if (numframe >= nb_frames) return 0;
    
    WORD nb_groupes = *(WORD*)(esi + 2);
    INT frame_size = nb_groupes * 8 + 8;
    esi += frame_size * numframe + 8;
    
    WORD ax = *(WORD*)edi;
    if (!(ax & INFO_ANIM)) return 0;
    
    edi += 14;
    *(INT*)(edi + 2) = (INT)esi;
    *(INT*)(edi + 6) = TimerRef;
    
    WORD size_info = *(WORD*)edi;
    edi += size_info + 2;
    
    WORD nb_points = *(WORD*)edi;
    edi += nb_points * 6 + 2 + 8 + 2;
    
    WORD obj_groupes = *(WORD*)(edi - 10);
    if (nb_groupes > obj_groupes) {
        nb_groupes = obj_groupes;
    }
    
    UBYTE *edx = esi;
    esi += 8;
    
    for (WORD cx = nb_groupes; cx > 0; cx--) {
        *(INT*)edi = *(INT*)esi;
        *(INT*)(edi + 4) = *(INT*)(esi + 4);
        esi += 8;
        edi += 38;
    }
    
    esi = edx + 2;
    AnimStepX = *(WORD*)esi;
    AnimStepY = *(WORD*)(esi + 2);
    AnimStepZ = *(WORD*)(esi + 4);
    
    esi += 6;
    AnimMasterRot = *(WORD*)esi;
    AnimStepAlpha = *(WORD*)(esi + 2);
    AnimStepBeta = *(WORD*)(esi + 4);
    AnimStepGamma = *(WORD*)(esi + 6);
    
    return 1;
}

static void PatchInterAngle(WORD &result)
{
    UBYTE *esi = (UBYTE*)Offset_Source;
    WORD cx = *(WORD*)esi;
    Offset_Source += 2;
    
    esi = (UBYTE*)Offset_Dest;
    WORD ax = *(WORD*)esi;
    Offset_Dest += 2;
    
    ax &= 1023;
    cx &= 1023;
    
    SHORT diff = ax - cx;
    if (diff != 0) {
        if (diff < -512) {
            diff += 1024;
        } else if (diff > 512) {
            diff -= 1024;
        }
        
        LONG temp = diff * (SHORT)CXSetInterAnim;
        temp /= (SHORT)nb_groupes;
        cx += temp;
    }
    
    result = cx & 1023;
}

static void PatchInterStep(WORD &result)
{
    UBYTE *esi = (UBYTE*)Offset_Source;
    WORD cx = *(WORD*)esi;
    Offset_Source += 2;
    
    esi = (UBYTE*)Offset_Dest;
    WORD ax = *(WORD*)esi;
    Offset_Dest += 2;
    
    SHORT diff = ax - cx;
    if (diff != 0) {
        LONG temp = diff * (SHORT)CXSetInterAnim;
        temp /= (SHORT)nb_groupes;
        cx += temp;
    }
    
    result = cx;
}

static WORD PatchType()
{
    UBYTE *esi = (UBYTE*)Offset_Dest;
    WORD ax = *(WORD*)esi;
    Offset_Dest += 2;
    Offset_Source += 2;
    return ax;
}

LONG SetInterAnimObjet(INT framedest, INT ptranimdest, INT ptrobj)
{
    UBYTE *esi = (UBYTE*)ptranimdest;
    UBYTE *edi = (UBYTE*)ptrobj;
    
    WORD nb_groupes = *(WORD*)(esi + 2);
    INT frame_size = nb_groupes * 8 + 8;
    esi += frame_size * framedest + 8;
    
    Offset_Dest = esi;
    WORD dx = *(WORD*)esi;
    
    WORD ax = *(WORD*)edi;
    if (!(ax & INFO_ANIM)) return 0;
    
    edi += 14;
    StartInfos = (INT)edi;
    
    INT ebx = *(INT*)edi;
    INT ebp = *(INT*)(edi + 4);
    
    if (ebx == 0) {
        ebp = dx;
        ebx = (INT)esi;
    }
    
    Offset_Source = ebx;
    
    WORD size_info = *(WORD*)(edi - 2);
    edi += size_info;
    
    WORD nb_points = *(WORD*)edi;
    edi += nb_points * 6 + 2 + 8 + 2;
    
    WORD obj_groupes = *(WORD*)(edi - 10);
    if (nb_groupes > obj_groupes) {
        nb_groupes = obj_groupes;
    }
    
    LONG eax = TimerRef - ebp;
    if (eax >= dx) {
        // Direct frame copy
        UBYTE *edx_ptr = esi;
        esi += 8;
        
        for (WORD cx = nb_groupes; cx > 0; cx--) {
            *(INT*)edi = *(INT*)esi;
            *(INT*)(edi + 4) = *(INT*)(esi + 4);
            esi += 8;
            edi += 38;
        }
        
        edi = (UBYTE*)StartInfos;
        *(INT*)edi = Offset_Dest;
        *(INT*)(edi + 4) = TimerRef;
        
        esi = edx_ptr + 2;
        AnimStepX = *(WORD*)esi;
        AnimStepY = *(WORD*)(esi + 2);
        AnimStepZ = *(WORD*)(esi + 4);
        
        esi += 6;
        AnimMasterRot = *(WORD*)esi;
        AnimStepAlpha = *(WORD*)(esi + 2);
        AnimStepBeta = *(WORD*)(esi + 4);
        AnimStepGamma = *(WORD*)(esi + 6);
        
        return 1;
    }
    
    // Interpolation
    WORD bx = dx;
    WORD bp = (WORD)eax;
    
    INT saved_offset_dest = Offset_Dest;
    
    Offset_Source += 8;
    Offset_Dest += 8;
    
    // First group special handling
    esi = (UBYTE*)Offset_Dest;
    AnimMasterRot = *(WORD*)esi;
    
    LONG temp = *(WORD*)(esi + 2) * bp / bx;
    AnimStepAlpha = (WORD)temp;
    
    temp = *(WORD*)(esi + 4) * bp / bx;
    AnimStepBeta = (WORD)temp;
    
    temp = *(WORD*)(esi + 6) * bp / bx;
    AnimStepGamma = (WORD)temp;
    
    Offset_Dest += 8;
    Offset_Source += 8;
    edi += 38;
    nb_groupes--;
    
    if (nb_groupes > 0) {
        CXSetInterAnim = nb_groupes;
        
        for (WORD cx = nb_groupes; cx > 0; cx--) {
            WORD type = PatchType();
            *(WORD*)edi = type;
            edi += 2;
            
            if (type == TYPE_ROTATE) {
                WORD result;
                PatchInterAngle(result);
                *(WORD*)edi = result;
                edi += 2;
                PatchInterAngle(result);
                *(WORD*)edi = result;
                edi += 2;
                PatchInterAngle(result);
                *(WORD*)edi = result;
                edi += 2;
            } else if (type == TYPE_TRANSLATE || type == TYPE_ZOOM) {
                WORD result;
                PatchInterStep(result);
                *(WORD*)edi = result;
                edi += 2;
                PatchInterStep(result);
                *(WORD*)edi = result;
                edi += 2;
                PatchInterStep(result);
                *(WORD*)edi = result;
                edi += 2;
            }
            
            edi += 12 + 18;
            CXSetInterAnim--;
        }
    }
    
    // Calculate step XYZ
    esi = (UBYTE*)saved_offset_dest;
    temp = *(WORD*)(esi + 2) * bp / bx;
    AnimStepX = (WORD)temp;
    
    temp = *(WORD*)(esi + 4) * bp / bx;
    AnimStepY = (WORD)temp;
    
    temp = *(WORD*)(esi + 6) * bp / bx;
    AnimStepZ = (WORD)temp;
    
    return 0;
}

LONG SetInterAnimObjet2(INT framedest, INT ptranimdest, INT ptrobj)
{
    UBYTE *esi = (UBYTE*)ptranimdest;
    UBYTE *edi = (UBYTE*)ptrobj;
    
    WORD nb_groupes = *(WORD*)(esi + 2);
    INT frame_size = (8 + nb_groupes * 8);
    esi += frame_size * framedest + 8;
    
    Offset_Dest = (INT)esi;
    WORD dx = *(WORD*)esi;
    
    WORD ax = *(WORD*)edi;
    if (!(ax & INFO_ANIM)) return 0;
    
    edi += 14;
    StartInfos = (INT)edi;
    
    INT ebx = *(INT*)edi;
    INT ebp = *(INT*)(edi + 4);
    
    if (ebx == 0) {
        ebp = dx;
        ebx = (INT)esi;
    }
    
    Offset_Source = ebx;
    
    WORD size_info = *(WORD*)(edi - 2);
    edi += size_info;
    
    WORD nb_points = *(WORD*)edi;
    edi += nb_points * 6 + 2 + 8 + 2;
    
    WORD obj_groupes = *(WORD*)(edi - 10);
    if (nb_groupes > obj_groupes) {
        nb_groupes = obj_groupes;
    }
    
    LONG eax = TimerRef - ebp;
    WORD bx = dx;
    WORD bp = (WORD)eax;
    
    if (eax >= dx) {
        // Direct frame copy
        esi += 8;
        
        for (WORD cx = nb_groupes; cx > 0; cx--) {
            *(INT*)edi = *(INT*)esi;
            *(INT*)(edi + 4) = *(INT*)(esi + 4);
            esi += 8;
            edi += 38;
        }
        
        edi = (UBYTE*)StartInfos;
        *(INT*)edi = Offset_Dest;
        
        return 1;
    }
    
    // Interpolation
    Offset_Dest += 16;
    Offset_Source += 16;
    edi += 38;
    nb_groupes--;
    
    if (nb_groupes > 0) {
        CXSetInterAnim = nb_groupes;
        
        for (WORD cx = nb_groupes; cx > 0; cx--) {
            WORD type = PatchType();
            *(WORD*)edi = type;
            edi += 2;
            
            if (type == TYPE_ROTATE) {
                WORD result;
                PatchInterAngle(result);
                *(WORD*)edi = result;
                edi += 2;
                PatchInterAngle(result);
                *(WORD*)edi = result;
                edi += 2;
                PatchInterAngle(result);
                *(WORD*)edi = result;
                edi += 2;
            } else if (type == TYPE_TRANSLATE || type == TYPE_ZOOM) {
                WORD result;
                PatchInterStep(result);
                *(WORD*)edi = result;
                edi += 2;
                PatchInterStep(result);
                *(WORD*)edi = result;
                edi += 2;
                PatchInterStep(result);
                *(WORD*)edi = result;
                edi += 2;
            }
            
            edi += 12 + 18;
            CXSetInterAnim--;
        }
    }
    
    return 0;
}

LONG SetInterDepObjet(INT framedest, INT ptranimdest, INT ptrobj)
{
    UBYTE *esi = (UBYTE*)ptranimdest;
    UBYTE *edi = (UBYTE*)ptrobj;
    
    WORD nb_groupes = *(WORD*)(esi + 2);
    INT frame_size = (8 + nb_groupes * 8);
    esi += frame_size * framedest + 8;
    
    Offset_Dest = (INT)esi;
    WORD dx = *(WORD*)esi;
    
    WORD ax = *(WORD*)edi;
    if (!(ax & INFO_ANIM)) return 0;
    
    edi += 14;
    StartInfos = (INT)edi;
    
    INT eax = *(INT*)edi;
    INT ebp = *(INT*)(edi + 4);
    
    if (eax == 0) {
        eax = (INT)esi;
        ebp = dx;
    }
    
    Offset_Source = eax;
    
    LONG timer_diff = TimerRef - ebp;
    if (timer_diff >= dx) {
        // Direct frame copy
        *(INT*)edi = Offset_Dest;
        *(INT*)(edi + 4) = TimerRef;
        
        esi += 2;
        AnimStepX = *(WORD*)esi;
        AnimStepY = *(WORD*)(esi + 2);
        AnimStepZ = *(WORD*)(esi + 4);
        
        esi += 6;
        AnimMasterRot = *(WORD*)esi;
        AnimStepAlpha = *(WORD*)(esi + 2);
        AnimStepBeta = *(WORD*)(esi + 4);
        AnimStepGamma = *(WORD*)(esi + 6);
        
        return 1;
    }
    
    // Interpolation
    WORD bx = dx;
    WORD bp = (WORD)timer_diff;
    
    UBYTE *ecx = (UBYTE*)Offset_Dest;
    
    Offset_Source += 8;
    Offset_Dest += 8;
    
    esi = (UBYTE*)Offset_Dest;
    AnimMasterRot = *(WORD*)esi;
    
    LONG temp = *(WORD*)(esi + 2) * bp / bx;
    AnimStepAlpha = (WORD)temp;
    
    temp = *(WORD*)(esi + 4) * bp / bx;
    AnimStepBeta = (WORD)temp;
    
    temp = *(WORD*)(esi + 6) * bp / bx;
    AnimStepGamma = (WORD)temp;
    
    Offset_Dest += 8;
    Offset_Source += 8;
    
    // Calculate step XYZ
    temp = *(WORD*)(ecx + 2) * bp / bx;
    AnimStepX = (WORD)temp;
    
    temp = *(WORD*)(ecx + 4) * bp / bx;
    AnimStepY = (WORD)temp;
    
    temp = *(WORD*)(ecx + 6) * bp / bx;
    AnimStepZ = (WORD)temp;
    
    return 0;
}

LONG StockInterAnim(INT ptranimbuf, INT ptrobj)
{
    UBYTE *esi = (UBYTE*)ptrobj;
    UBYTE *edi = (UBYTE*)ptranimbuf;
    
    WORD ax = *(WORD*)esi;
    if (!(ax & INFO_ANIM)) return 0;
    
    esi += 14;
    
    *(INT*)(esi + 4) = TimerRef;
    *(INT*)esi = (INT)edi;
    
    WORD size_info = *(WORD*)(esi - 2);
    esi += size_info;
    
    WORD nb_points = *(WORD*)esi;
    esi += nb_points * 6 + 2;
    
    WORD nb_groupes = *(WORD*)esi;
    
    edi += 8;
    esi += 8 + 2;
    
    for (WORD cx = nb_groupes; cx > 0; cx--) {
        *(INT*)edi = *(INT*)esi;
        *(INT*)(edi + 4) = *(INT*)(esi + 4);
        edi += 8;
        esi += 38;
    }
    
    return 1;
}

void CopyInterAnim(INT ptrobjs, INT ptrobjd)
{
    UBYTE *esi = (UBYTE*)ptrobjs;
    UBYTE *edi = (UBYTE*)ptrobjd;
    
    WORD ax = *(WORD*)esi;
    if (!(ax & INFO_ANIM)) return;
    
    ax = *(WORD*)edi;
    if (!(ax & INFO_ANIM)) return;
    
    esi += 14;
    edi += 14;
    
    *(INT*)edi = *(INT*)esi;
    *(INT*)(edi + 4) = *(INT*)(esi + 4);
    
    WORD size_info_s = *(WORD*)(esi - 2);
    esi += size_info_s;
    WORD nb_points_s = *(WORD*)esi;
    esi += nb_points_s * 6 + 2;
    WORD nb_groupes_s = *(WORD*)esi;
    
    WORD size_info_d = *(WORD*)(edi - 2);
    edi += size_info_d;
    WORD nb_points_d = *(WORD*)edi;
    edi += nb_points_d * 6 + 2;
    WORD nb_groupes_d = *(WORD*)edi;
    
    WORD cx = (nb_groupes_s <= nb_groupes_d) ? nb_groupes_s : nb_groupes_d;
    
    esi += 8 + 2;
    edi += 8 + 2;
    
    for (; cx > 0; cx--) {
        *(INT*)edi = *(INT*)esi;
        *(INT*)(edi + 4) = *(INT*)(esi + 4);
        esi += 30;
        edi += 30;
    }
}
