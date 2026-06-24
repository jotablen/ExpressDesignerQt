# COC Algorithm Comparison: ODs vs ExpressDesignerQt

## ODs `CalculaPtoAnal` (Funciones_SMS.cpp:2742)

```
Input: pto_P1 (point on WF1), vect (ray direction, normalized),
       index1 (n1), index2 (n2), fOpt_Road_Len (remaining OPL after first surface),
       WF_To (target wavefront)

Variables:
  t  = physical distance from pto_P1 along vect  (NOT optical path)
  X  = pto_P1 + vect * t                          (physical trial point)
  CP = Find_CP_Repaired(X, WF_To)                 (closest point on WF_To whose normal passes through X)
  out = X - CP                                    (vector from CP to X)
  C(t) = |dot(out, normal(CP))| * n2              (optical path from WF_To to X)

Secant iteration:
  f(t) = (n1 * t) + C(t) - fOpt_Road_Len = 0
  → finds t such that total OPL matches target

Steps:
  1. ta = fOpt_Road_Len / 2.0                     (physical)
  2. Ca = C(ta), fa = n1*ta + Ca - fOpt_Road_Len
  3. tb = ta + delta_t (delta_t = fOpt_Road_Len/20)
  4. Iterate secant: tc = ta - fa*(tb-ta)/(fb-fa)
  5. Fallback: adaptive stepping forward/reverse
```

## ExpressDesignerQt `calcCarthesianPoint2D` (CarthesianOvalOperation.cpp:29)

```
Input: p1 (point on WF1), n1 (normal direction, normalized),
       n1_idx (refractive index of WF1), curve2 (target WF as SISLCurve),
       n2_idx (refractive index of WF2), OpticalPathLen (target OPL)

Variables:
  t  = ??? ambiguous — used as BOTH physical and optical
  dist1 = t * n1_idx                        ← ???
  X = p1 + n1 * dist1                       ← physical point, but dist1 is problematic
  p2 = curve2.closestPoint(X, &ct)          ← geometric closest (NOT repaired!)
  n2 = curve2.normalAt(p2)                  ← unflipped geometric normal
  d1 = dot(X - p1, n1)                      ← signed distance along n1
  d2 = dot(X - p2, n2)                      ← signed distance along n2
  OPL = d1 * n1_idx + d2 * n2_idx           ← calculated OPL (but d1,d2 may not pass normals)

Problem 1: t is ambiguous
  ta = OpticalPathLen / 2.0    — starts as OPTICAL path
  dist1 = ta * n1_idx           — multiplies by n1 (now it's OPL × n1?)
  X = p1 + n1 * dist1           — physical point: p1 + n1 × (OPL/2 × n1)
  → if n1≠1, X is in the wrong place

  In ODs: ta = fOpt_Road_Len / 2.0 (physical!)
  X = pto_P1 + vect * ta         (physical!)

Problem 2: closestPoint is not repaired
  ODs: Find_CP_Repaired(X, WF_To) ensures the normal of CP passes through X
  Ours: curve2.closestPoint(X) only finds geometric closest (no normal check)

Problem 3: normal direction
  ODs: uses PointNormalToVector(pto_Tmp) which respects WF orientation
  Ours: uses curve2.normalAt(p2) which is unflipped geometric normal

## Why first point works but others don't

The first point (refPoint) uses repairedPoint which correctly finds CP whose normal passes
through the reference point. The OPL constant C is computed correctly.

But `calcCarthesianPoint2D` which computes all other points uses:
  - closestPoint() without repair → wrong CP, wrong normal
  - t as optical path instead of physical distance → wrong position of trial X
  - normalAt() without WF orientation → wrong OPL

## Required fixes

1. **t must be physical distance**: X = p1 + n1 * t, OPL contributes n1 * t
2. **Use Find_CP_Repaired instead of closestPoint** in oplError loop
3. **Use WF-oriented normal** (curve.normal(t, flipped) instead of normalAt)
4. **Add missing SISLCurve method**: pointWithNormalThrough or equivalent