# NOTAS

## Fase 0: ambiente

- Linker: o compilador deixa um buraco no .o onde a função é chamada, e o linker preenche esse buraco com o endereço da definição. Sem definição, dá `undefined reference`.
- Decisão: MSYS2 para rodar o C++ do programa de cálculo no PC. O firmware é compilado pelo PlatformIO, não pelo MSYS2.

## Fase 1: LED, resistor, millis()

**Circuito:** GPIO 18 -> resistor -> LED -> GND
**Dados:** fonte 3,3 V, queda no LED ~2 V, então sobram 1,3 V no resistor.
**Fórmula:** I = V / R (corrente em A, tensão em V, R em ohm)

| R     | Conta       | I       |
| ----- | ----------- | ------- |
| 220 Ω | 1,3 / 220   | 5,9 mA  |
| 10 kΩ | 1,3 / 10000 | 0,13 mA |
| 22 Ω  | 1,3 / 22    | 59 mA   |
| 2 Ω   | 1,3 / 2     | 650 mA  |

- Regra: com a mesma tensão, R maior = corrente menor.
- Por que o LED precisa de resistor: para fixar a corrente. Sem ele, nada na conta limita.

**Achar R para 10 mA:**

1. I = 10 / 1000 = 0,01 A
2. R = V / I = 1,3 / 0,01 = 130 Ω
3. Confiro: 1,3 / 130 = 0,01 A = 10 mA (bateu)
4. 130 Ω não é comum, uso 150 Ω: 1,3 / 150 = 8,67 mA (menos corrente, lado seguro)

- Ao arredondar R, vou para mais ohm. Sempre confiro voltando a conta.

**Pisca sem delay():**

- `delay()` congela o programa, e um aperto dentro dele se perde.
- Com `millis()`: se agora - último >= intervalo, inverto o estado e guardo o `millis()` desse instante (não zero).
- Só uma linha escreve no pino (`digitalWrite`), e ela repete o estado guardado.

**Errei:** achei que 10 kΩ era "mais largo" e passava mais corrente. Ohm mede o quanto o resistor segura a corrente.

## Fase 2: botão

- Pino sem nada ligado flutua, e a leitura não é confiável.
- **Pull-up** (10 kΩ do 3V3 ao pino, botão do pino ao GND):
  - Solto: quase sem corrente, 0 V caem no resistor, 3,3 V chegam ao pino, lê **1**.
  - Apertado: 0,33 mA, 3,3 V caem no resistor, sobra 0 V no pino, lê **0**.
  - Lógica invertida: apertado = 0.
- **Nível** (`port4`, o que o pino vale agora) vs **borda** (mudou de 1 pra 0, só existe comparando com a leitura anterior).
- **Bounce:** o contato quica por alguns ms. Guardo o momento da última **mudança** da leitura e só aceito a borda se passaram >= 50 ms.
- **τ = R x C** = 10000 x 0,0000001 = 1 ms. Muito menor que o quique, então o hardware sozinho não basta e preciso do filtro em software.
- Decisão: filtro em hardware (100 nF) + software (50 ms).

**Errei:**

- Achei que botão solto "corta a ligação" e dá 0. Na verdade fica flutuando.
- Liguei os dois fios no mesmo terminal do botão (`1.l` e `1.r` são o mesmo terminal).
- `millis()` não é data, são ms desde que a placa ligou.
- `buttonPressed` guarda a leitura **anterior**, atualizada em toda passada do `loop()`, e não só quando há evento.

**Pendente (Fase 7):** botão real com e sem o capacitor, contando leituras falsas. Testar também o pico de ruído com o botão solto.
