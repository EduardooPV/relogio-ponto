## Fase 0:

- Linker: o compilador deixa um buraco no .o onde a função é chamada, e o linker preenche esse buraco.
- Errei: achei que 10k era mais forte, mas na verdade quanto maior o número, menor a passagem de energia, resultando em um led mais fraco.
- Decisão: MSYS2 em vez de WSL porque ele roda o software de calculo. Ele serve para rodar o C++ (e não para o firmware, que é compilado pela extenssão do PlatformIO).

## Fase 1: resistor do LED (conta feita à mão)

Circuito: GPIO 18 -> resistor -> LED -> GND
Dados: fonte 3,3 V, queda no LED (Vf) ~2 V
Tensão sobre o resistor: 3,3 - 2 = 1,3 V
Fórmula: I = V / R

| R     | Conta       | I       |
| ----- | ----------- | ------- |
| 220 Ω | 1,3 / 220   | 5,9 mA  |
| 10 kΩ | 1,3 / 10000 | 0,13 mA |
| 22 Ω  | 1,3 / 22    | 59 mA   |
| 2 Ω   | 1,3 / 2     | 650 mA  |

Regra: com a mesma tensão, R maior = corrente menor.
Errei: achei que 10 kΩ era "mais largo" e passava mais corrente. Ohm mede o quanto o resistor segura.
Por que o LED precisa de resistor: para fixar a corrente, se não tiver um resistor na conta nada limita a corrente.

### Achar R pra uma corrente desejada (ida e volta)

Ida (acho o R):

1. Corrente desejada em A: 10 mA / 1000 = 0,01 A
2. Tensão sobre o resistor: 3,3 - 2 = 1,3 V
3. R = V / I = 1,3 / 0,01 = 130 ohm

Volta (confiro):
I = V / R = 1,3 / 130 = 0,01 A = 10 mA (bateu)

Valor comercial: 130 ohm não é comum, uso 150 ohm.
I = 1,3 / 150 = 8,67 mA (menor que 10 mA, então seguro)

Regras:

- Na fórmula, corrente sempre em A e tensão em V, e o R sai em ohm.
- Ao arredondar R, vou pro lado de mais ohm (corrente cai).
- Sempre conferir voltando a conta.