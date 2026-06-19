# PropagateWF — ExpressDesigner vs Ovals Designer
## Comparación de algoritmos de discretización e intersección

---

## 1. Flujo general

| Paso | Ovals Designer (SISLDrink) | ExpressDesigner (actual) |
|------|---------------------------|-------------------------|
| 1 | Evalúa WF en `n` puntos con `evaluate()` | Igual — `wfCurve.evaluateAll(numPoints)` |
| 2 | Calcula normal continua con `normal(t, flipped)` | Igual — `wfCurve.normal(t, flipWF)` |
| 3 | **Intersección rayo ↔ superficie** | **Intersección curva ↔ plano** |
| 4 | Calcula OPL, Snell, propaga | Igual |
| 5 | Crea `CurveObject` y lo añade al proyecto | Igual |

---

## 2. Diferencia clave: intersección

### 2.1 Ovals Designer (discretizado)
```pascal
// Paso 1: Discretiza la superficie a M puntos
surfPts := surfCurve.evaluateAll(numPoints)

// Paso 2: Para cada rayo, busca intersección rayo ↔ segmento
for i := 0 to numPoints-1 do begin
  srcPt := wfPts[i]
  norm  := wfCurve.normal(t, flipWF)
  // Iterar segmentos de la superficie discretizada
  surfHit, hitDist := RayCurveIntersection(srcPt, norm, surfPts)
  if hitDist >= 0 then begin
    OPL := n1 * hitDist
    // Normal en el punto de impacto: usar segmento donde cayó
    surfNormal := segmentNormal(surfPts, hitSeg)
    // Snell, propagar...
  end
end
```

**Características:**
- La superficie se evalúa a `numPoints` puntos (típicamente 100-200)
- La intersección es **rayo ↔ segmento recto** sobre la polilínea discretizada
- La normal en el impacto se calcula a partir del segmento donde cayó (normal discreta del segmento)
- **Funciona bien en la práctica porque la discretización de 100-200 puntos es suficientemente densa** para curvas ópticas típicas

### 2.2 ExpressDesigner (actual — curva ↔ plano)
```cpp
// Paso 1: Evalúa denso la superficie a 200 puntos
surfDense := surfCurve.evaluateAll(200)

// Paso 2: Para cada rayo, define un plano y busca intersección
for i := 0 to numPoints-1:
  srcPt := wfPts[i]
  norm  := wfCurve.normal(t, flipWF)
  // Define plano que pasa por srcPt con normal = norm
  // Intersecta la curva con ese plano (todos los segmentos)
  surfHit, hitDist := surfCurve.planeIntersection(srcPt, norm)
  if hitDist >= 0:
    OPL := n1 * hitDist
    // Normal en el punto de impacto: continua (no discretizada)
    surfNormal := surfCurve.normalAt(surfHit)
    // Snell, propagar...
```

**Características:**
- `planeIntersection()` evalúa la curva a 200 puntos internamente
- La intersección es **plano ↔ segmento recto** sobre la polilínea densa
- La normal en el impacto se calcula con `normalAt()` (continua sobre la curva)
- **Produce resultados idénticos a la versión discretizada porque ambos usan 200 puntos densos**

---

## 3. ¿Son equivalentes?

| Aspecto | Rayo ↔ Segmento (ODs) | Plano ↔ Curva (ExpressDesigner) |
|---------|----------------------|-------------------------------|
| Precisión de intersección | Buena (100-200 pts) | Igual (200 pts densos) |
| Normal en el impacto | Segmento discreto | Continua (`normalAt`) |
| Resultado visual | Correcto para óptica | Correcto para óptica |
| Casos límite (curvas muy curvas) | Puede fallar con pocos puntos | Ídem si los puntos de control son pocos |
| Rendimiento | O(N × M) | O(N × 200) ≈ igual |

**Conclusión:** Ambos algoritmos son **matemáticamente equivalentes** cuando la densidad de puntos es la misma (200 puntos en ExpressDesigner ≈ 100-200 en ODs). La diferencia real está en la **normal en el impacto**: ExpressDesigner usa `normalAt()` continua, ODs usaba la normal del segmento discreto. Para sistemas ópticos de alta precisión, la normal continua es superior.

---

## 4. Nota sobre SISL real vs polilínea

| Sistema | Intersección | Precisión |
|---------|-------------|-----------|
| ODs con SISL real (`s1850`) | Curva NURBS ↔ plano (intersección exacta sobre la curva paramétrica) | **Máxima** — sin discretización |
| ExpressDesigner actual | Polilínea densa (200 pts) ↔ plano | **Buena** — limitada por la densidad de evaluación |
| ExpressDesigner con `USE_SISL=ON` | Curva NURBS ↔ plano (usando `s1850` de SISL) | **Máxima** — igual que ODs |

Para alcanzar la precisión exacta del SISL real, ExpressDesigner necesitaría compilar con `USE_SISL=ON` y vincular la biblioteca SISL. La implementación actual con polilínea densa es una aproximación válida para prototipado.

---

## 5. Recomendación

1. **Mantener `planeIntersection()` con 200 puntos** — produce resultados equivalentes a la versión discretizada de ODs
2. **Para precisión máxima**, compilar con `USE_SISL=ON` y usar `s1850` de SISL para intersección exacta curva-plano sobre NURBS
3. **La normal continua (`normalAt`) es correcta y debe mantenerse** — es superior a la normal de segmento discreto

---

*Generado: Junio 2026 — ExpressDesignerQt*