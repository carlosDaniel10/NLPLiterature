# Modelo de sentimiento fastText (NLPFront)

## 1. Qué es

`AnalizadorFastText` clasifica el sentimiento de un texto con un modelo supervisado de
[fastText](https://github.com/facebookresearch/fastText) (Meta) de tres clases y convierte las
probabilidades en un puntaje en [-1, 1]:

```
puntaje = P(positivo) - P(negativo)     (acotado a [-1, 1])
```

- Puntaje > 0 → arista verde (positiva); puntaje < 0 → arista roja (negativa), igual que con el
  léxico.
- Texto vacío o error de predicción → 0.0 (queda como arista neutra).

## 2. Tipo de modelo y arquitectura

- fastText **v0.9.2** (tag fijo en CMake), modo **supervised** (`model_name::sup`).
- Clasificador lineal (softmax) sobre una **bolsa de n-gramas**: cada palabra se representa como
  la suma de los vectores de sus subpalabras (char n-gramas de 3 a 6 caracteres) más el vector de
  la palabra completa. Esto captura morfología del español (amor, amores, amoroso...) sin un
  vocabulario cerrado: las palabras no vistas se reconstruyen por sus subpalabras.
- 3 etiquetas: `__label__positivo`, `__label__negativo`, `__label__neutral`.

## 3. Datos de entrenamiento (etiquetado débil)

- **Fuente**: las 25 obras XML-TEI de `Obras/` (Valle-Inclán, García Lorca, Valera, Galdós, etc.).
- **Segmentación**: parlamentos (`sp`) en drama y párrafos/versos (`p`/`l`) en narrativa; se
  descarta el `teiHeader`.
- **Etiqueta**: cada unidad se puntúa con el léxico (`sentimientos_es.txt`):
  puntaje ≥ +0.25 → positivo; puntaje ≤ −0.25 → negativo; resto → neutral.
- **Filtros**: mínimo 4 palabras por unidad; máximo 250 muestras por obra; los neutrales se
  limitan a 3×(positivas+negativas)+50 mediante submuestreo uniforme (paso fijo).
- **Cifras del modelo actual**: 6151 muestras escritas (1459 positivas, 683 negativas, 4009
  neutrales), 8176 palabras distintas tras `minCount=2`, léxico de 321 palabras.

## 4. Hiperparámetros

| Parámetro   | Valor        | Nota                                          |
|-------------|--------------|-----------------------------------------------|
| `model`     | `sup`        | clasificación supervisada                     |
| `loss`      | `softmax`    | pérdida softmax multiclase                    |
| `dim`       | 100          | dimensión de los vectores                     |
| `lr`        | 0.5          | tasa de aprendizaje (decrece linealmente)     |
| `epoch`     | 5            | épocas (ajustable por línea de comandos)      |
| `wordNgrams`| 2            | bigramas de palabras                          |
| `minCount`  | 2            | frecuencia mínima de palabra                  |
| `minn/maxn` | 3/6          | longitud de subpalabras (valores por defecto) |
| `bucket`    | 2 000 000    | espacio de hash de subpalabras (por defecto)  |
| `thread`    | 4            | hilos de entrenamiento                        |

- Entrenamiento: ~1 minuto (~220 000 palabras/segundo/hilo).
- Pérdida final ≈ 0.59 (la pérdida aleatoria con 3 clases es ln 3 ≈ 1.10).

## 5. Cuantización

- El `.bin` sin cuantizar pesa ~800 MB porque guarda la matriz de entrada completa con el bucket
  de 2 millones de subpalabras (2M × 100 × 4 bytes).
- Se cuantiza con product quantization (`dsub=4`; `qout=false` porque la matriz de salida solo
  tiene 3 filas y no admite cuantización) y queda `sentimientos_modelo.ftz` de ~50 MB.
- La pérdida de precisión por cuantización es mínima en la práctica.

## 6. Archivos

| Archivo | Generado por | Propósito | En git |
|---|---|---|---|
| `Obras/*.xml` | corpus externo | datos de entrenamiento y de análisis | sí |
| `NLPFront/sentimientos_es.txt` | manual | léxico de polaridad (alimenta el etiquetado débil y el `AnalizadorLexico`) | sí (recurso qrc) |
| `NLPFront/sentimientos_modelo.train.txt` | `entrenarmodelo` | ejemplos de entrenamiento (`__label__ + texto`) | no (gitignore) |
| `NLPFront/sentimientos_modelo.ftz` | `entrenarmodelo` | modelo cuantizado que carga la app | no (gitignore) |
| `NLPFront/cmake/parchear_fasttext.cmake` | manual | parches de fastText v0.9.2: `#include <cstdint>` para GCC 13 y renombre del `main` del CLI para que no choque con el de la app | sí |

## 7. Uso

Entrenar por consola (ver también `AGENTS.md`):

```powershell
NLPFront\build\herramientas\entrenarmodelo.exe Obras NLPFront\sentimientos_es.txt NLPFront\sentimientos_modelo.bin [epochs]
```

El entrenador escribe `<salida>.train.txt`, entrena, cuantiza a `<salida>.ftz` y borra el `.bin`
gigante.

### Reentrenar desde la aplicación

Botón **"Reentrenar modelo"** → diálogo `DialogoEntrenar`, con dos modos:

- **Proceso completo**: carpeta de obras + léxico del recurso integrado de la app
  (`:/sentimientos_es.txt`). Genera `<salida>.train.txt` y entrena.
- **Solo train.txt**: usa un archivo ajustado a mano; antes de entrenar valida y reporta el
  conteo de etiquetas por clase.

Ajustes disponibles: epochs, dimensión de vectores y bucket de subpalabras. El entrenamiento
corre en un hilo aparte (la interfaz no se congela) con barra de progreso y registro de log. Al
terminar, la app carga el nuevo `.ftz` y ofrece reanalizar la obra abierta con el nuevo modelo.

En la app: al iniciar busca `sentimientos_modelo.ftz` (y luego `.bin`) junto al ejecutable y uno
o dos niveles arriba (esto cubre los árboles de build de desarrollo). Si existe, el combo queda
en FastText; si no, se usa el léxico. Al cambiar el analizador con una obra ya cargada hay que
volver a abrirla para recalcular las aristas.

## 8. Evaluación

Pérdida de entrenamiento final ≈ 0.59. Ejemplos sobre el modelo cuantizado:

| Frase | positivo | negativo | neutral |
|---|---|---|---|
| "te amo con todo el corazón" | 0.72 | 0.13 | 0.15 |
| "te odio y te desprecio maldito" | 0.30 | 0.70 | 0.00 |
| "el camino es largo y la noche fría" | 0.04 | 0.03 | 0.93 |

## 9. Limitaciones

- El etiquetado débil hereda los sesgos del léxico: no captura ironía ni sarcasmo y las
  negaciones solo se manejan con una ventana simple.
- Las unidades (parlamentos y párrafos completos) pueden mezclar varios sentimientos.
- No hay conjunto de validación: solo se observa la pérdida de entrenamiento.
- El corpus es de un solo idioma y época literaria; el modelo no generaliza a otros dominios.

## 10. Psoibles mejora

**Validación y métricas por clase**: reservar 15-20% de las muestras y medir
   precision/recall/F1 por clase (fastText expone `test()`); con eso se podrían ajustar
   hiperparámetros con autotune.
**Etiquetado manual curado**: etiquetar a mano 1-2 mil unidades y reentrenar; el formato
   `__label__ + texto` ya lo permite sin cambios de código.
**Segmentación por oración**: dividir párrafos en oraciones antes de etiquetar para que cada
   muestra tenga un solo sentimiento.
**Ampliar el léxico**: `sentimientos_es.txt` con mas vocabulario con
   intensidades mejoraría a la vez el etiquetado débil y el analizador léxico.
**Barrido de hiperparámetros**: probar dim 50/100/200, epoch 5/10/25, wordNgrams 1/2/3 y lr
   0.1/0.5 sobre la validación.
**Usar la confianza del modelo**: ponderar el registro en la arista por P(pos)+P(neg) o
   descartar predicciones de entropía alta; hoy solo se usa P(pos)−P(neg).
**Modelo por dominio**: comparar un modelo global con modelos por autor o género.
**Modelo más liviano**: probar `qnorm`, bucket menor (p. ej. 500k) o dim 50 y medir el
   impacto en precisión y en el tiempo de arranque de la app (~1-2 s con 50 MB).

