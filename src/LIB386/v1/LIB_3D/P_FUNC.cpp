#include "LIB_3D.H"

// External variables
extern DWORD TimerRef;
extern WORD P_SinTab[];

// Public variables
DWORD Distance = 0;
DWORD EX0 = 0;
DWORD EY0 = 0;

// Real-time value structure
struct RealValueStruct {
    WORD startValue;
    WORD endValue;
    WORD timeValue;
    DWORD timerRef;
};

void InitRealValue(DWORD startValue, DWORD endValue, DWORD timeValue, DWORD ptrstruct) {
    RealValueStruct* s = (RealValueStruct*)ptrstruct;
    s->startValue = (WORD)startValue;
    s->endValue = (WORD)endValue;
    s->timeValue = (WORD)timeValue;
    s->timerRef = TimerRef;
}

void InitRealAngle(DWORD startValue, DWORD endValue, DWORD timeValue, DWORD ptrstruct) {
    RealValueStruct* s = (RealValueStruct*)ptrstruct;
    s->startValue = (WORD)(startValue & 1023);
    s->endValue = (WORD)(endValue & 1023);
    s->timeValue = (WORD)timeValue;
    s->timerRef = TimerRef;
}

void InitRealAngleConst(DWORD startValue, DWORD endValue, DWORD timeValue, DWORD ptrstruct) {
    RealValueStruct* s = (RealValueStruct*)ptrstruct;
    WORD start = (WORD)(startValue & 1023);
    WORD end = (WORD)(endValue & 1023);
    
    s->startValue = start;
    s->endValue = end;
    
    WORD delta = start - end;
    delta <<= 6;
    if ((SHORT)delta < 0) delta = -delta;
    delta >>= 6;
    
    DWORD adjustedTime = (delta * timeValue) >> 8;
    s->timeValue = (WORD)adjustedTime;
    s->timerRef = TimerRef;
}

LONG GetRealValue(DWORD ptrstruct) {
    RealValueStruct* s = (RealValueStruct*)ptrstruct;
    
    if (s->timeValue == 0) {
        return (LONG)(SHORT)s->endValue;
    }
    
    DWORD elapsed = TimerRef - s->timerRef;
    if (elapsed >= s->timeValue) {
        s->timeValue = 0;
        return (LONG)(SHORT)s->endValue;
    }
    
    SHORT delta = s->endValue - s->startValue;
    LONG result = (delta * elapsed) / s->timeValue;
    return result + s->startValue;
}

LONG GetRealAngle(DWORD ptrstruct) {
    RealValueStruct* s = (RealValueStruct*)ptrstruct;
    
    if (s->timeValue == 0) {
        return s->endValue;
    }
    
    DWORD elapsed = TimerRef - s->timerRef;
    if (elapsed >= s->timeValue) {
        s->timeValue = 0;
        return s->endValue;
    }
    
    SHORT delta = s->endValue - s->startValue;
    
    // Handle angle wrapping
    if (delta < -512) {
        delta += 1024;
    } else if (delta > 512) {
        delta -= 1024;
    }
    
    LONG result = (delta * elapsed) / s->timeValue;
    return result + s->startValue;
}

LONG BoundRegleTrois(DWORD valeur1, DWORD valeur2, DWORD nbStep, DWORD step) {
    if (step <= 0) {
        return valeur1;
    }
    if (step >= nbStep) {
        return valeur2;
    }
    
    LONG delta = valeur2 - valeur1;
    return valeur1 + (delta * step) / nbStep;
}

LONG RegleTrois32(DWORD valeur1, DWORD valeur2, DWORD nbStep, DWORD step) {
    if (nbStep <= 0) {
        return valeur1;
    }
    
    LONG delta = valeur2 - valeur1;
    return valeur1 + (delta * step) / nbStep;
}

DWORD Sqr(DWORD value) {
    if (value <= 3) {
        return (value == 0) ? 0 : 1;
    }
    
    DWORD original = value;
    DWORD result = 1;
    DWORD remainder = 0;
    
    // Find highest bit position
    DWORD bitPos = 0;
    DWORD temp = value;
    while (temp) {
        bitPos++;
        temp >>= 1;
    }
    
    // Calculate shift amount
    DWORD shiftCount = 33 - bitPos;
    shiftCount &= ~1;
    
    // Shift value to position
    remainder = (original >> (32 - shiftCount)) & ((1 << shiftCount) - 1);
    value <<= shiftCount;
    
    DWORD pairs = bitPos / 2;
    remainder--;
    
    // Newton-Raphson style iteration
    while (pairs > 0) {
        remainder = (remainder << 2) | ((value >> 30) & 3);
        value <<= 2;
        result <<= 2;
        
        if (remainder >= result) {
            result++;
            remainder -= result;
            if (remainder >= 0) {
                result++;
                result >>= 1;
                pairs--;
                continue;
            }
            remainder += result;
        }
        result >>= 1;
        pairs--;
    }
    
    return result;
}

DWORD Distance2D(DWORD px0, DWORD py0, DWORD px1, DWORD py1) {
    LONG dx = px1 - px0;
    LONG dy = py1 - py0;
    
    DWORD distSq = (dx * dx) + (dy * dy);
    return Sqr(distSq);
}

DWORD Distance3D(DWORD px0, DWORD py0, DWORD pz0, DWORD px1, DWORD py1, DWORD pz1) {
    LONG dx = px1 - px0;
    LONG dy = py1 - py0;
    LONG dz = pz1 - pz0;
    
    DWORD distSq = (dx * dx) + (dy * dy) + (dz * dz);
    return Sqr(distSq);
}

DWORD GetAngle(DWORD x0, DWORD z0, DWORD x1, DWORD z1) {
    LONG dx = x1 - x0;
    LONG dz = z1 - z0;
    LONG savedX = dx;
    LONG savedZ = dz;
    
    // Take absolute values for calculation
    if (dx < 0) dx = -dx;
    if (dz < 0) dz = -dz;
    
    DWORD flags = 0;
    
    // Ensure dx >= dz for better precision
    if (dx < dz) {
        LONG temp = dx;
        dx = dz;
        dz = temp;
        temp = savedX;
        savedX = savedZ;
        savedZ = temp;
        flags |= 1;
    }
    
    DWORD distSq = (dx * dx) + (dz * dz);
    DWORD dist = Sqr(distSq);
    Distance = dist;
    
    if (dist == 0) {
        return 0;
    }
    
    // Calculate sine value
    LONG sinValue = (savedZ << 14) / dist;
    
    // Binary search in sine table
    WORD* sinTab = P_SinTab + 384;  // Start at 90 degrees offset
    WORD* start = sinTab;
    WORD* end = sinTab + 256;
    
    while (end - start > 1) {
        WORD* mid = start + (end - start) / 2;
        if (sinValue <= *mid) {
            end = mid;
        } else {
            start = mid;
        }
    }
    
    // Choose closer value
    if (*start == sinValue) {
        // Exact match
    } else {
        LONG avgValue = (*start + *end) / 2;
        if (sinValue > avgValue) {
            start = end;
        }
    }
    
    // Calculate angle index
    DWORD angle = (start - (P_SinTab + 256)) / sizeof(WORD);
    
    // Adjust for quadrant
    if (savedX < 0) {
        angle = -angle;
    }
    
    if (flags & 1) {
        angle = 256 - angle;
    }
    
    return angle & 1023;
}

void Rot2D(DWORD coorx, DWORD coory, DWORD angle) {
    if (angle == 0) {
        EX0 = coorx;
        EY0 = coory;
        return;
    }
    
    angle &= 1023;
    
    // Get sin and cos values from table
    SHORT sinVal = (SHORT)P_SinTab[angle];
    SHORT cosVal = (SHORT)P_SinTab[(angle + 256) & 1023];
    
    // Perform rotation: X' = X*cos - Y*sin, Y' = X*sin + Y*cos
    LONG newX = ((LONG)coorx * cosVal - (LONG)coory * sinVal) >> 15;
    LONG newY = ((LONG)coorx * sinVal + (LONG)coory * cosVal) >> 15;
    
    EX0 = newX;
    EY0 = newY;
}
