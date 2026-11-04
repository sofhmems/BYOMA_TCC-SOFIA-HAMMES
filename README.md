# Sistema Automatizado de Análise Comparativa da Sustentabilidade Têxtil

Sistema automatizado de análise comparativa da sustentabilidade de tecidos, com
aquisição de dados por ESP32 e modelos 3D desenvolvidos no FreeCAD.

## Estrutura do repositório

```
.
├── codigos/
│   ├── 01-sensor-particulas/
│   │   └── esp32/esp32.ino
│   ├── 02-sistema-desgaste/
│   │   └── sistema_desgaste/sistema_desgaste.ino
│   ├── 03-movimento-vertical/
│   │   └── sistema_mov_vertical/sistema_mov_vertical.ino
│   ├── 04-integracao-lixa-esteira/
│   │   └── codigo_integracao_lixa_esteira/codigo_integracao_lixa_esteira.ino
│   └── 05-testes/
│       ├── teste_led/
│       ├── teste_motor_de_passo/
│       ├── teste_motor_esteira/
│       ├── teste_motor_lixa/
│       ├── teste_sensor_luminosidade/
│       └── teste_sensor_particulas/
├── freecad/                      Modelos 3D do projeto (.FCStd)
├── .gitignore
└── README.md
```

Cada sketch fica em uma pasta com o mesmo nome do arquivo `.ino`, como exige a
Arduino IDE.

## 1. Sensor de partículas — `codigos/01-sensor-particulas/`

Firmware do ESP32 que mede a quantidade de partículas liberadas pelo tecido
(sensor óptico GP2Y1010AU0F) e estima a variação de densidade antes e depois do
desgaste.

| Item | Valor |
| ---- | ----- |
| Placa | ESP32 |
| Sensor | GP2Y1010AU0F |
| Pino de leitura | `A0` |
| Pino do LED do sensor | `5` |
| Envio | HTTP POST JSON a cada 5 s |

Bibliotecas: `WiFi`, `HTTPClient`, `ArduinoJson`.

**Antes de gravar:** ajuste `SSID`, `PASSWORD` e `SERVER_URL` no topo do arquivo.

## 2. Sistema de desgaste — `codigos/02-sistema-desgaste/`

Controla o forno de ensaio e o acesso ao posto de teste via Modbus TCP.

| Item | Valor |
| ---- | ----- |
| Placa | ESP32 / ESP8266 |
| Protocolo | Modbus TCP |
| Temperatura (ADC) | GPIO `34` |
| Biblioteca | `modbus-esp8266` |

Registros Modbus usados: temperatura, setpoint do forno, estado do forno,
solicitação de acesso, LED de acesso liberado e código/ID do usuário.

**Credenciais:** copie `secrets.h.example` para `secrets.h` na mesma pasta do
`.ino` e preencha com os dados da sua rede. O `secrets.h` é ignorado pelo Git.

## 3. Movimento vertical — `codigos/03-movimento-vertical/`

Sketch da estrutura de movimentação vertical.

| Item | Valor |
| ---- | ----- |
| Placa | ESP32 |
| LED de sinalização | GPIO `18` |

> Estado atual: o sketch apenas pisca o LED a cada 1 s (esqueleto para o
> acionamento do motor).

## 4. Integração lixa + esteira — `codigos/04-integracao-lixa-esteira/`

Ciclo automático que desce a lixa, aciona o motor de lixa e sobe a lixa.

| Item | Valor |
| ---- | ----- |
| Driver do motor de passo — STEP | GPIO `32` |
| Driver do motor de passo — DIR | GPIO `33` |
| Relé da lixa | GPIO `25` (ativo em LOW) |
| Tempo de descida/subida | 1500 ms |
| Tempo de lixa ligada | 1500 ms |

## 5. Testes — `codigos/05-testes/`

Sketches de bancada usados para validar cada componente isoladamente.

| Sketch | Componente | Pinos |
| ------ | ---------- | ----- |
| `teste_led` | LED do GP2Y1010AU0F + ADC (ESP-IDF) | LED `27`, ADC1_CH6 (`34`) |
| `teste_motor_de_passo` | Sensor de luminosidade TSL2561 (I2C) | SDA/SCL padrão |
| `teste_motor_esteira` | Relé do motor da esteira | `25` |
| `teste_motor_lixa` | Motor de passo da lixa | STEP `32`, DIR `33` |
| `teste_sensor_luminosidade` | Sensor de luminosidade TSL2561 (I2C) | SDA/SCL padrão |
| `teste_sensor_particulas` | Relé (acionamento liga/desliga) | `26` |

Bibliotecas: `SparkFunTSL2561` e `Wire` (sensores de luminosidade); `teste_led`
usa as APIs nativas do ESP-IDF (`driver/adc`, `driver/gpio`, FreeRTOS).

> Observação: dois sketches têm conteúdo diferente do nome da pasta —
> `teste_motor_de_passo` contém o código do sensor de luminosidade e
> `teste_sensor_particulas` contém um teste de relé. Os arquivos foram mantidos
> como estavam no diretório original.

## 6. Modelos 3D — `freecad/`

Arquivos `.FCStd` do FreeCAD com as peças do projeto:

- `bastidor para caixa-guia.FCStd` — bastidor de sustentação da caixa-guia
- `caixa-guia para tecido.FCStd` — caixa-guia por onde passa o tecido
- `haste guia para estrutura de movimentação vertical.FCStd` — haste de guia da
  estrutura de movimentação vertical
- `suporte motor de lixa.FCStd` — suporte do motor de lixa

Para abrir: FreeCAD 0.20 ou superior → `Arquivo > Abrir`.

## Aviso sobre dados sensíveis

Nenhuma senha de rede ou de conta real está versionada neste repositório.
Os valores de exemplo são placeholders e devem ser substituídos localmente.
