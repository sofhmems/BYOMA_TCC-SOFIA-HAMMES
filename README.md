# Sustentabilidade Têxtil — TCC

Sistema automatizado de análise comparativa da sustentabilidade de tecidos, com
aquisição de dados por ESP32, backend em Node.js e interface web.

## Estrutura do repositório

```
.
├── codigos/
│   ├── 01-sensor-particulas/     Firmware do sensor de partículas (ESP32)
│   ├── 02-sistema-desgaste/      Firmware do sistema de desgaste (ESP32/ESP8266 + Modbus)
│   └── 03-sistema-web/           Backend Node.js + interface web
├── freecad/                      Modelos 3D do projeto (.FCStd)
└── README.md
```

## 1. Sensor de partículas — `codigos/01-sensor-particulas/`

Firmware do ESP32 responsável por medir a quantidade de partículas liberadas pelo
tecido (sensor óptico GP2Y1010AU0F) e estimar a variação de densidade antes e
depois do desgaste.

- Placa: ESP32
- Sensor: GP2Y1010AU0F (saída analógica)
- Comunicação: HTTP POST em JSON para `/api/esp32/dados`
- Intervalo de envio: 5 s

**Antes de gravar:** ajuste `SSID`, `PASSWORD` e `SERVER_URL` no topo do arquivo
com os dados da sua rede.

## 2. Sistema de desgaste — `codigos/02-sistema-desgaste/`

Firmware que controla o forno de ensaio e o acesso ao posto de teste via Modbus TCP.

- Placa: ESP32 / ESP8266
- Protocolo: Modbus TCP (`ModbusIP_ESP8266`)
- Entradas: temperatura (ADC), setpoint, código de acesso e ID do usuário
- Saídas: comando do forno e LED de acesso liberado

**Credenciais:** copie `secrets.h.example` para `secrets.h` e preencha com os
dados da sua rede. O arquivo `secrets.h` é ignorado pelo Git.

## 3. Sistema web — `codigos/03-sistema-web/`

Backend Express + interface web (login, cadastro, dashboard, histórico e resultados).

```bash
cd codigos/03-sistema-web
npm install
npm start
```

Acesse `http://localhost:3000`.

Variáveis de ambiente (opcional, veja `.env.example`):

| Variável         | Descrição                                  |
| ---------------- | ------------------------------------------ |
| `PORT`           | Porta do servidor (padrão `3000`)          |
| `SESSION_SECRET` | Chave de assinatura do cookie de sessão    |
| `ADMIN_PASSWORD` | Senha do usuário `admin`                   |
| `USER_PASSWORD`  | Senha do usuário `usuario`                 |

> As credenciais de demonstração (`admin` / `123456` e `usuario` / `textil2024`)
> existem apenas para testes locais. Troque antes de qualquer uso real.

### Endpoints principais

| Método | Rota                  | Descrição                            |
| ------ | --------------------- | ------------------------------------ |
| POST   | `/api/login`          | Autenticação                         |
| POST   | `/api/cadastrar`      | Criação de conta                     |
| POST   | `/api/analisar`       | Cálculo de sustentabilidade          |
| GET    | `/api/historico`      | Histórico do usuário                 |
| GET    | `/api/historico/csv`  | Exportação do histórico em CSV       |
| POST   | `/api/esp32/dados`    | Recebimento das leituras do ESP32    |
| GET    | `/api/esp32/status`   | Status do dispositivo                |

## 4. Modelos 3D — `freecad/`

Arquivos `.FCStd` do FreeCAD com as peças e montagens do projeto
(suportes, guias, caixa guia, encaixes e capas do motor).

Para abrir: FreeCAD 0.20 ou superior → `Arquivo > Abrir`.

## Aviso sobre dados sensíveis

Nenhuma senha de rede ou de conta real está versionada neste repositório.
Os valores de exemplo são placeholders e devem ser substituídos localmente.
