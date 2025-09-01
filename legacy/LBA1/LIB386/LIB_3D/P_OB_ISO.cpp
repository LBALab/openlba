#include "LIB_3D.H"

// External variables from M_TRIGO
extern LONG CameraXr, CameraYr, CameraZr;
extern LONG X0, Y0, Z0;
extern LONG XCentre, YCentre;
extern LONG lAlpha, lBeta, lGamma;
extern LONG NormalXLight, NormalYLight, NormalZLight;
extern LONG LMatriceWorld;
extern WORD TypeProj;
extern LONG KFactor, LFactorX, LFactorY;
extern LONG LMat00, LMat01, LMat02;
extern LONG LMat10, LMat11, LMat12;
extern LONG LMat20, LMat21, LMat22;
extern LONG TabMat;
extern WORD compteur;

// External variables from LIB_GRAF
extern WORD NbPolyPoints;
extern WORD TabPoly;
extern WORD TypePoly;

// Public variables
WORD FlagLight = 1;
WORD List_Point[500*3];      // Xp Yp Zrot
WORD List_Normal[500];
WORD List_Anim_Point[500*3]; // Xr Yr Zr
WORD List_Entity[5000];
WORD List_Tri[500*8];
WORD ScreenXmin = 0, ScreenYmin = 0;
WORD ScreenXmax = 0, ScreenYmax = 0;
WORD NbPoints = 0;

// Private variables
static WORD TotalEntite = 0;
static WORD ZMax = 0;
static WORD NbGroupes = 0;
static WORD Infos = 0;
static WORD Count1 = 0;
static LONG PointeurListTri = 0;
static LONG StartDI = 0;
static LONG NextDI = 0;
static LONG ListGroupe = 0;
static LONG StartInfos = 0;
static LONG OffsetDefPoint = 0;
static LONG OffsetListNormal = 0;
static LONG Save1 = 0, Save2 = 0, Save3 = 0;
static LONG Ptr1 = 0, Ptr2 = 0;
static LONG PosXWr = 0, PosYWr = 0, PosZWr = 0;

// Jump table for entity types
typedef void (*EntityFunc)();
static EntityFunc TabJump_2[] = { aff_E_LIGNE, aff_E_POLY, aff_E_SPHERE };

// External function declarations
extern void ComputePoly_A();
extern void ComputeSphere_A();
extern void FillVertic_A();
extern void Line_A();
extern void RotMatW();
extern void RotMatIndex2();
extern void WorldRot();
extern void LongWorldRot();
extern void Rot();
extern void Proj_3D();
extern void Proj_ISO();
extern void RotList();
extern void TransRotList();

// Forward declarations
static void RotateGroupe();
static void TranslateGroupe();
static void AnimNuage();
static void RotateNuage();
static void ComputeAnimNormal();
static void aff_E_POLY();
static void aff_E_LIGNE();
static void aff_E_SPHERE();
static void nextaff();

void PatchObjet(LONG ptrobj) {
    WORD* esi = (WORD*)ptrobj;
    
    if (!(esi[0] & INFO_ANIM)) return;
    
    WORD nbBytesToSkip = esi[7];
    esi = (WORD*)((BYTE*)esi + nbBytesToSkip + 16);
    
    WORD nbPoints = esi[0];
    esi++;
    
    esi = (WORD*)((BYTE*)esi + nbPoints * 6);
    
    WORD nbGroups = esi[0];
    esi++;
    
    if (--nbGroups == 0) return;
    
    for (WORD i = 0; i < nbGroups; i++) {
        esi = (WORD*)((BYTE*)esi + 38);
        WORD orggroupe = esi[3];
        esi[3] = (orggroupe * 36) / 38;
    }
}

LONG AffObjetIso(LONG xwr, LONG ywr, LONG zwr, LONG palpha, LONG pbeta, LONG pgamma, LONG ptrobj) {
    lAlpha = palpha;
    lBeta = pbeta;
    lGamma = pgamma;
    
    ScreenXmin = ScreenYmin = 32767;
    ScreenXmax = ScreenYmax = -32767;
    
    // World rotation of object origin
    if (TypeProj == TYPE_3D) {
        LongWorldRot(xwr, ywr, zwr);
        PosXWr = X0 - CameraXr;
        PosYWr = Y0 - CameraYr;
        PosZWr = Z0 - CameraZr;
    } else {
        PosXWr = xwr;
        PosYWr = ywr;
        PosZWr = zwr;
    }
    
    TotalEntite = 0;
    PointeurListTri = (LONG)List_Tri;
    
    WORD* esi = (WORD*)ptrobj;
    Infos = esi[0];
    
    WORD nbBytesToSkip = esi[7];
    esi = (WORD*)((BYTE*)esi + nbBytesToSkip + 16);
    
    // Point cloud/normal face/normal point rotation
    if (Infos & INFO_ANIM) {
        AnimNuage(esi);
    } else {
        RotateNuage(esi);
    }
    
    // Process polygons
    WORD* edi = List_Entity;
    WORD nbPolys = esi[0];
    esi++;
    
    if (nbPolys > 0) {
        Count1 = nbPolys;
        
        while (Count1 > 0) {
            StartDI = (LONG)edi;
            
            LONG matInfo = *(LONG*)esi;
            esi++;
            
            BYTE matType = matInfo & 0xFF;
            BYTE nbPoints = (matInfo >> 8) & 0xFF;
            WORD colors = matInfo >> 16;
            
            WORD zMax = -32000;
            
            if (matType >= 9) { // MAT_GOURAUD
                matType -= 2;
                edi[0] = matType;
                edi[1] = nbPoints;
                *(WORD*)(edi+2) = colors;
                esi++;
                edi += 2;
                
                Ptr1 = (LONG)edi;
                
                for (WORD i = 0; i < nbPoints; i++) {
                    WORD normalIdx = esi[0];
                    WORD pointIdx = esi[1];
                    esi += 2;
                    
                    WORD intensity = List_Normal[normalIdx] + (colors & 0xFF);
                    edi[0] = intensity;
                    
                    *(LONG*)(edi+1) = *(LONG*)&List_Point[pointIdx*3];
                    WORD z = List_Point[pointIdx*3+2];
                    
                    if (z > zMax) zMax = z;
                    edi += 3;
                }
            } else if (matType >= 7) { // MAT_FLAT
                matType -= 7;
                edi[0] = matType;
                edi[1] = nbPoints;
                
                WORD normalIdx = esi[1];
                WORD intensity = colors + List_Normal[normalIdx];
                *(WORD*)(edi+1) = intensity;
                esi += 2;
                edi += 2;
                
                Ptr1 = (LONG)edi;
                
                for (WORD i = 0; i < nbPoints; i++) {
                    WORD pointIdx = *esi++;
                    *(LONG*)(edi+1) = *(LONG*)&List_Point[pointIdx*3];
                    WORD z = List_Point[pointIdx*3+2];
                    
                    if (z > zMax) zMax = z;
                    edi += 3;
                }
            } else { // MAT_TRISTE->MAT_TRAME
                *(LONG*)edi = matInfo;
                esi++;
                edi += 2;
                
                Ptr1 = (LONG)edi;
                
                for (WORD i = 0; i < nbPoints; i++) {
                    WORD pointIdx = *esi++;
                    *(LONG*)(edi+1) = *(LONG*)&List_Point[pointIdx*3];
                    WORD z = List_Point[pointIdx*3+2];
                    
                    if (z > zMax) zMax = z;
                    edi += 3;
                }
            }
            
            NextDI = (LONG)edi;
            ZMax = zMax;
            
            // Backface culling test
            WORD* vertices = (WORD*)Ptr1;
            LONG crossProd = (LONG)(vertices[2] - vertices[8]) * (vertices[5] - vertices[1]) -
                           (LONG)(vertices[1] - vertices[7]) * (vertices[4] - vertices[2]);
            
            if (crossProd >= 0) {
                // Add to entity list
                TotalEntite++;
                WORD* tri = (WORD*)PointeurListTri;
                tri[0] = ZMax;
                tri[1] = E_POLY;
                *(LONG*)(tri+2) = StartDI;
                PointeurListTri += 8;
            }
            
            edi = (WORD*)NextDI;
            Count1--;
        }
    }
    
    // Process lines
    WORD nbLines = esi[0];
    esi++;
    
    if (nbLines > 0) {
        TotalEntite += nbLines;
        WORD* edx = (WORD*)PointeurListTri;
        
        for (WORD i = 0; i < nbLines; i++) {
            *(LONG*)edi = *(LONG*)esi;
            
            WORD pointIdx1 = esi[2];
            WORD pointIdx2 = esi[3];
            esi += 4;
            
            *(LONG*)(edi+2) = *(LONG*)&List_Point[pointIdx1*3];
            *(LONG*)(edi+4) = *(LONG*)&List_Point[pointIdx2*3];
            
            WORD z1 = List_Point[pointIdx1*3+2];
            WORD z2 = List_Point[pointIdx2*3+2];
            WORD zMin = (z1 < z2) ? z1 : z2;
            
            edx[0] = zMin;
            edx[1] = E_LIGNE;
            *(LONG*)(edx+2) = (LONG)edi;
            edx += 4;
            edi += 6;
        }
        
        PointeurListTri = (LONG)edx;
    }
    
    // Process spheres
    WORD nbSpheres = esi[0];
    esi++;
    
    if (nbSpheres > 0) {
        TotalEntite += nbSpheres;
        WORD* edx = (WORD*)PointeurListTri;
        
        for (WORD i = 0; i < nbSpheres; i++) {
            *(LONG*)edi = *(LONG*)esi;
            LONG radiusPoint = *(LONG*)(esi+2);
            esi += 4;
            
            WORD radius = radiusPoint & 0xFFFF;
            WORD pointIdx = radiusPoint >> 16;
            
            edi[2] = radius;
            *(LONG*)(edi+3) = *(LONG*)&List_Point[pointIdx*3];
            WORD z = List_Point[pointIdx*3+2];
            edi[5] = z;
            
            edx[0] = z;
            edx[1] = E_SPHERE;
            *(LONG*)(edx+2) = (LONG)edi;
            edx += 4;
            edi += 6;
        }
        
        PointeurListTri = (LONG)edx;
    }
    
    // Sort entities by Z
    if (TotalEntite > 1) {
        // QuickSort implementation for entity sorting
        WORD* sortList = (WORD*)List_Tri;
        WORD count = TotalEntite - 1;
        
        // Simple bubble sort for small arrays
        for (WORD i = 0; i < count; i++) {
            for (WORD j = 0; j < count - i; j++) {
                if (sortList[j*4] > sortList[(j+1)*4]) {
                    // Swap entities
                    LONG temp1 = *(LONG*)&sortList[j*4];
                    LONG temp2 = *(LONG*)&sortList[j*4+2];
                    *(LONG*)&sortList[j*4] = *(LONG*)&sortList[(j+1)*4];
                    *(LONG*)&sortList[j*4+2] = *(LONG*)&sortList[(j+1)*4+2];
                    *(LONG*)&sortList[(j+1)*4] = temp1;
                    *(LONG*)&sortList[(j+1)*4+2] = temp2;
                }
            }
        }
    }
    
    // Display entities
    if (TotalEntite == 0) {
        ScreenXmax = ScreenYmax = ScreenXmin = ScreenYmin = -1;
        return 1;
    }
    
    WORD* entityList = (WORD*)List_Tri;
    Count1 = TotalEntite;
    Ptr1 = (LONG)entityList;
    
    while (Count1 > 0) {
        WORD entityType = entityList[1];
        WORD* entityData = (WORD*)*(LONG*)(entityList+2);
        Ptr1 += 8;
        
        // Call appropriate rendering function
        TabJump_2[entityType]();
        
        entityList = (WORD*)Ptr1;
        Count1--;
    }
    
    return 0;
}

static void aff_E_POLY() {
    WORD* esi = (WORD*)*(LONG*)(((WORD*)Ptr1)-2);
    
    LONG typeInfo = *(LONG*)esi;
    BYTE type = typeInfo & 0xFF;
    BYTE nbPoints = (typeInfo >> 8) & 0xFF;
    WORD colors = typeInfo >> 16;
    
    TypePoly = type;
    NbPolyPoints = nbPoints;
    
    // Copy vertex data
    WORD* src = esi + 2;
    WORD* dst = (WORD*)TabPoly;
    for (WORD i = 0; i < nbPoints * 3; i++) {
        *dst++ = *src++;
    }
    
    if (ComputePoly_A()) {
        FillVertic_A(colors, type);
    }
    
    nextaff();
}

static void aff_E_LIGNE() {
    WORD* esi = (WORD*)*(LONG*)(((WORD*)Ptr1)-2);
    
    WORD colors = (esi[1] << 8) | esi[0];
    LONG point1 = *(LONG*)(esi+2);
    LONG point2 = *(LONG*)(esi+4);
    
    WORD x0 = point1 & 0xFFFF;
    WORD y0 = point1 >> 16;
    WORD x1 = point2 & 0xFFFF;
    WORD y1 = point2 >> 16;
    
    Line_A(x0, y0, x1, y1, colors);
    
    nextaff();
}

static void aff_E_SPHERE() {
    WORD* esi = (WORD*)*(LONG*)(((WORD*)Ptr1)-2);
    
    BYTE type = esi[0] & 0xFF;
    WORD colors = esi[0] >> 8;
    WORD radius = esi[2];
    LONG center = *(LONG*)(esi+3);
    WORD z = esi[5];
    
    WORD x = center & 0xFFFF;
    WORD y = center >> 16;
    
    Save1 = type;
    Save2 = colors;
    
    // Adjust radius for projection
    if (TypeProj == TYPE_3D) {
        radius = (radius * LFactorX) / (z + KFactor);
    } else {
        radius = (radius * 34) >> 9; // Isometric scale
    }
    
    // Update screen bounds
    WORD xMin = x - radius, xMax = x + radius;
    WORD yMin = y - radius, yMax = y + radius;
    
    if (xMax > ScreenXmax) ScreenXmax = xMax;
    if (xMin < ScreenXmin) ScreenXmin = xMin;
    if (yMax > ScreenYmax) ScreenYmax = yMax;
    if (yMin < ScreenYmin) ScreenYmin = yMin;
    
    if (ComputeSphere_A(x, y, radius)) {
        FillVertic_A(colors, type);
    }
    
    nextaff();
}

static void nextaff() {
    // Continue to next entity
}

static void RotateGroupe() {
    // Group rotation implementation
    // Complex matrix operations for group hierarchies
}

static void TranslateGroupe() {
    // Group translation implementation
}

static void AnimNuage() {
    WORD* esi = (WORD*)*(LONG*)(Save1);
    
    WORD nbPoints = esi[0];
    esi++;
    NbPoints = nbPoints;
    
    OffsetDefPoint = (LONG)esi;
    esi = (WORD*)((BYTE*)esi + nbPoints * 6);
    
    NbGroupes = esi[0];
    esi++;
    ListGroupe = (LONG)esi;
    
    // Process group 0
    Ptr1 = (LONG)TabMat;
    Save1 = (LONG)esi;
    RotateGroupe();
    
    esi = (WORD*)(Save1 + 38);
    Save1 = (LONG)esi;
    
    // Process other groups
    WORD groupCount = NbGroupes - 1;
    if (groupCount > 0) {
        Count1 = groupCount;
        Ptr1 = (LONG)TabMat + 36;
        
        while (Count1 > 0) {
            LONG animInfo = *(LONG*)(esi+4);
            WORD animType = animInfo & 0xFFFF;
            LONG stepX = animInfo >> 16;
            
            LONG stepInfo = *(LONG*)(esi+6);
            WORD stepY = stepInfo & 0xFFFF;
            LONG stepZ = stepInfo >> 16;
            
            if (animType == TYPE_ROTATE) {
                RotateGroupe();
            } else if (animType == TYPE_TRANSLATE) {
                TranslateGroupe();
            }
            
            Ptr1 += 36;
            esi = (WORD*)(Save1 + 38);
            Save1 = (LONG)esi;
            Count1--;
        }
    }
    
    // Project point list
    Count1 = NbPoints;
    WORD* srcPoints = List_Anim_Point;
    WORD* dstPoints = List_Point;
    
    for (WORD i = 0; i < NbPoints; i++) {
        LONG x = (LONG)(SHORT)srcPoints[0] + PosXWr;
        LONG y = (LONG)(SHORT)srcPoints[1] + PosYWr;
        LONG z = (LONG)(SHORT)srcPoints[2] + PosZWr;
        z = -z;
        
        if (TypeProj == TYPE_3D) {
            // 3D projection
            z += KFactor;
            if (z <= 0) z = 0x7FFFFFFF;
            
            LONG projX = (x * LFactorX) / z + XCentre;
            LONG projY = (-y * LFactorY) / z + YCentre;
            
            if (projX > 0xFFFF) projX = 0x7FFF;
            if (projY > 0xFFFF) projY = 0x7FFF;
            if (z > 0xFFFF) z = 0x7FFF;
            
            dstPoints[0] = (WORD)projX;
            dstPoints[1] = (WORD)projY;
            dstPoints[2] = (WORD)z;
        } else {
            // Isometric projection
            LONG isoX = ((z + x) * 24) >> 9;
            LONG isoY = (((x - z) * 12) + (-y * 30)) >> 9;
            
            dstPoints[0] = (WORD)(isoX + XCentre);
            dstPoints[1] = (WORD)(isoY + YCentre);
            dstPoints[2] = (WORD)(-z);
        }
        
        // Update screen bounds
        if (dstPoints[0] < ScreenXmin) ScreenXmin = dstPoints[0];
        if (dstPoints[0] > ScreenXmax) ScreenXmax = dstPoints[0];
        if (dstPoints[1] < ScreenYmin) ScreenYmin = dstPoints[1];
        if (dstPoints[1] > ScreenYmax) ScreenYmax = dstPoints[1];
        
        srcPoints += 3;
        dstPoints += 3;
    }
    
    ComputeAnimNormal();
}

static void ComputeAnimNormal() {
    WORD* esi = (WORD*)Save1;
    WORD nbNormals = esi[0];
    esi++;
    
    if (nbNormals == 0) return;
    
    WORD* normals = List_Normal;
    Ptr1 = (LONG)normals;
    Ptr2 = (LONG)TabMat;
    
    Count1 = NbGroupes;
    WORD* groupInfo = (WORD*)(ListGroupe + 18);
    Save1 = (LONG)groupInfo;
    
    while (Count1 > 0) {
        WORD groupNormals = groupInfo[0];
        if (groupNormals > 0) {
            compteur = groupNormals;
            
            // Setup matrix for normal transformation
            LONG* matrix = (LONG*)Ptr2;
            
            // Process normals for this group
            while (compteur > 0) {
                LONG nx = (LONG)(SHORT)esi[0];
                LONG ny = (LONG)(SHORT)esi[1];
                LONG nz = (LONG)(SHORT)esi[2];
                WORD range = esi[3];
                esi += 4;
                
                // Transform normal and compute lighting
                LONG intensity = (nx * NormalXLight + ny * NormalYLight + nz * NormalZLight) >> 14;
                if (intensity < 0) intensity = 0;
                intensity /= range;
                
                *(WORD*)Ptr1 = (WORD)intensity;
                Ptr1 += 2;
                compteur--;
            }
        }
        
        groupInfo = (WORD*)(Save1 + 38);
        Save1 = (LONG)groupInfo;
        Ptr2 += 36;
        Count1--;
    }
}

static void RotateNuage() {
    WORD* esi = (WORD*)Save1;
    Save1 = (LONG)esi;
    
    RotMatW(); // Rotate world matrix
    
    esi = (WORD*)Save1;
    WORD nbPoints = esi[0];
    esi++;
    NbPoints = nbPoints;
    
    WORD* dstPoints = List_Point;
    Count1 = nbPoints;
    Ptr1 = (LONG)dstPoints;
    
    for (WORD i = 0; i < nbPoints; i++) {
        WORD x = esi[0], y = esi[1], z = esi[2];
        esi += 3;
        
        // Rotate point
        Rot(x, y, z);
        
        // Transform to camera space
        WORD camX = X0 - PosXWr;
        WORD camY = Y0 - PosYWr;
        WORD camZ = Z0 + PosZWr;
        camZ = -camZ;
        
        // Project point
        Save2 = camZ;
        Proj_ISO(camX, camY);
        camZ = Save2;
        
        dstPoints[0] = camX;
        dstPoints[1] = camY;
        dstPoints[2] = camZ;
        
        // Update screen bounds
        if (camX < ScreenXmin) ScreenXmin = camX;
        if (camX > ScreenXmax) ScreenXmax = camX;
        if (camY < ScreenYmin) ScreenYmin = camY;
        if (camY > ScreenYmax) ScreenYmax = camY;
        
        dstPoints += 3;
        Count1--;
    }
    
    // Compute static normals
    WORD nbNormals = esi[0];
    esi++;
    
    if (nbNormals > 0) {
        WORD* normals = List_Normal;
        Count1 = nbNormals;
        Ptr1 = (LONG)normals;
        
        while (Count1 > 0) {
            WORD nx = esi[0], ny = esi[1], nz = esi[2], range = esi[3];
            esi += 4;
            
            Rot(nx, ny, nz);
            
            LONG intensity = ((LONG)X0 * NormalXLight + (LONG)Y0 * NormalYLight + (LONG)Z0 * NormalZLight) / range;
            if (intensity < 0) intensity = 0;
            
            *(WORD*)Ptr1 = (WORD)intensity;
            Ptr1 += 2;
            Count1--;
        }
    }
}
