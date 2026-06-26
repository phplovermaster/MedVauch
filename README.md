# MedVauch


Sistema inteligente de lembrete e gerenciamento de medicamentos desenvolvido com **Arduino Uno**, **Display LCD 16x2** e **Buzzer Piezoelétrico**.

## ✨ Funcionalidades

- Cadastro de medicamentos via Monitor Serial
- Validação de medicamentos permitidos
- Agendamento automático por intervalo de horas
- Exibição do horário atual e próximo medicamento no LCD 16x2
- Alertas sonoros utilizando buzzer piezoelétrico
- Indicação visual através de LED
- Reprodução de melodia personalizada durante os alarmes
- Ordenação automática dos medicamentos cadastrados
- Prevenção de cadastros duplicados
- Remoção automática após confirmação do usuário

## 🛠 Tecnologias Utilizadas

| Tecnologia | Finalidade |
|------------|------------|
| Arduino Uno | Microcontrolador principal |
| C++ | Desenvolvimento do firmware |
| Arduino IDE | Compilação e gravação |
| LiquidCrystal | Controle do LCD |
| LCD 16x2 | Interface visual |
| Buzzer Piezoelétrico | Alertas sonoros |

## 📋 Pré-requisitos

- Arduino Uno
- LCD 16x2
- Buzzer
- LED Verde
- Protoboard
- Jumpers
- Arduino IDE

## 🚀 Instalação

```bash
https://github.com/phplovermaster/MedVauch.git
cd medvault
```

## ⚙ Configuração

Configure os medicamentos permitidos e parâmetros do sistema diretamente no arquivo `MedVault.ino`.

## ▶ Execução

1. Conecte o Arduino.
2. Faça upload do código.
3. Abra o Monitor Serial em 9600 baud.
4. Cadastre medicamentos no formato:

```text
Dipirona 4
```

## 📂 Estrutura do Projeto

```text
MedVault/
├── README.md
├── MedVault.ino
├── docs/
└── assets/
```

## 📄 Licença

MIT License

https://www.tinkercad.com/things/1ZbO3kmbpdv-projeto-israely/editel?returnTo=https%3A%2F%2Fwww.tinkercad.com%2Fdashboard&sharecode=hIyC2OYVco5ASlwdU0pQ6GODN0EFcFVtd1RP4OoCpmw

![CAPTURA DA MONTAGEM](img/Captura%20de%20tela%202026-06-14%20184826.png)
