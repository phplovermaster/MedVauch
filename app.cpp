#include <LiquidCrystal.h>

const int GREEN_LED = 7;
const int PIEZO = 6; // O buzzer do primeiro código foi mapeado para o pino do PIEZO (6)
const int MAX_MEDS = 10;

const unsigned long SIMULATED_HOUR_MS = 5000;
const unsigned long VIBRATION_DURATION_MS = 3000;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

struct Medication {
    String name;
    int hour;
    bool alerted;
    bool taken;
};

Medication medications[MAX_MEDS];

int totalMedications = 0;
int currentHour = 0;

unsigned long previousMillis = 0;
unsigned long motorStartMillis = 0;
unsigned long tempMessageUntil = 0;

bool isVibrating = false;
bool forceScreenUpdate = true;

int lastDisplayedHour = -1;

// ==================== NOVA MUSIQUINHA INTEGRADA ====================
int tempo = 120; // Ajustado o tempo base para a música fluir melhor sem travar o sistema
char notes[] = "eeeeeeegcde fffffeeeeddedg";
int duration[] = {1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2};
int totalNotes = sizeof(notes) - 1;

int currentNoteIndex = 0;
unsigned long noteStartMillis = 0;
unsigned long noteTotalDuration = 0; 
bool melodyPlaying = false;
bool isSilentPeriod = false; // Controla o pequeno intervalo entre as notas
// ===================================================================

String builtinMeds[] = {
    "Dorflex",
    "Dipirona",
    "Paracetamol",
    "Ibuprofeno",
    "Amoxicilina"
};

const int totalBuiltinMeds = 5;

// Função auxiliar adaptada do primeiro código (sem loops internos ou delays)
int getToneFrequency(char note) {
    char notesName[] = { 'c', 'd', 'e', 'f', 'g' };
    int tones[] = { 261, 293, 329, 349, 392 };
    
    for (int i = 0; i < 5; i++) {
        if (note == notesName[i]) {
            return tones[i];
        }
    }
    return 0; // Retorna 0 se for espaço ou nota inválida
}

void setup() {
    lcd.begin(16, 2);

    Serial.begin(9600);
    Serial.setTimeout(50);

    pinMode(GREEN_LED, OUTPUT);
    pinMode(PIEZO, OUTPUT);

    digitalWrite(PIEZO, LOW);

    lcd.print("MedVault Ready");
    delay(1500);
    lcd.clear();

    Serial.println("=== MEDVAULT ===");
    Serial.println("Digite: [nome] [horas]");
}

bool isBuiltinMedication(String medName) {
    for (int i = 0; i < totalBuiltinMeds; i++) {
        if (medName.equalsIgnoreCase(builtinMeds[i])) {
            return true;
        }
    }
    return false;
}

void showTemporaryMessage(String msg) {
    lcd.clear();
    lcd.print(msg);
    tempMessageUntil = millis() + 1500;
    forceScreenUpdate = true;
}

void sortMedications() {
    for (int i = 0; i < totalMedications - 1; i++) {
        for (int j = i + 1; j < totalMedications; j++) {
            if (medications[j].hour < medications[i].hour) {
                Medication temp = medications[i];
                medications[i] = medications[j];
                medications[j] = temp;
            }
        }
    }
}

void removeMedication(int index) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Tomou remedio?");

    Serial.println("\nTomou o remedio? (s/n)");
    while (Serial.available() == 0) {}

    String tomou = Serial.readString();
    tomou.trim();

    if (tomou.equalsIgnoreCase("sim") || tomou.equalsIgnoreCase("s")) {
        Serial.println("Medicamento tomado.");
    } else {
        Serial.println("Medicamento nao tomado.");
    }

    for (int i = index; i < totalMedications - 1; i++) {
        medications[i] = medications[i + 1];
    }

    totalMedications--;
    digitalWrite(GREEN_LED, LOW);

    if (totalMedications == 0) {
        currentHour = 0;
        previousMillis = millis();
        lcd.clear();
        lcd.print("Sem horarios");
    }

    forceScreenUpdate = true;
}

void registerMedication() {
    if (totalMedications >= MAX_MEDS) {
        showTemporaryMessage("Limite atingido");
        return;
    }

    String medName = Serial.readStringUntil(' ');
    medName.trim();

    int intervalHour = Serial.parseInt();

    while (Serial.available() > 0) {
        Serial.read();
    }

    if (!isBuiltinMedication(medName)) {
        showTemporaryMessage("Medic invalido");
        Serial.println("Medicamento invalido.");
        return;
    }

    if (intervalHour <= 0) {
        showTemporaryMessage("Tempo invalido");
        return;
    }

    int targetHour = (currentHour + intervalHour) % 24;

    for (int i = 0; i < totalMedications; i++) {
        if (medications[i].hour == targetHour && medications[i].name.equalsIgnoreCase(medName)) {
            showTemporaryMessage("Duplicado");
            return;
        }
    }

    medications[totalMedications].name = medName;
    medications[totalMedications].hour = targetHour;
    medications[totalMedications].alerted = false;
    medications[totalMedications].taken = false;

    totalMedications++;
    sortMedications();

    showTemporaryMessage("Salvo!");

    Serial.print("Agendado: ");
    Serial.print(medName);
    Serial.print(" ");
    Serial.print(targetHour);
    Serial.println("h");
}

void displayNextMedication() {
    if (millis() < tempMessageUntil || isVibrating) {
        return;
    }

    if (totalMedications == 0) {
        if (forceScreenUpdate) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Sem remedios");
            lcd.setCursor(0, 1);
            lcd.print("Relogio parado");
            forceScreenUpdate = false;
        }
        return;
    }

    if (!forceScreenUpdate && lastDisplayedHour == currentHour) {
        return;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Atual: ");

    if (currentHour < 10) {
        lcd.print("0");
    }
    lcd.print(currentHour);
    lcd.print(":00");

    int nextIndex = -1;
    for (int i = 0; i < totalMedications; i++) {
        if (medications[i].hour >= currentHour && !medications[i].alerted) {
            nextIndex = i;
            break;
        }
    }

    if (nextIndex == -1 && totalMedications > 0) {
        nextIndex = 0;
    }

    lcd.setCursor(0, 1);
    if (nextIndex != -1) {
        lcd.print(medications[nextIndex].name);
        lcd.print(" ");
        lcd.print(medications[nextIndex].hour);
        lcd.print("h");
    }

    lastDisplayedHour = currentHour;
    forceScreenUpdate = false;
}

void handleAlertsAndMotor() {
    for (int i = 0; i < totalMedications; i++) {
        if (medications[i].hour == currentHour && !medications[i].alerted && !isVibrating) {
            medications[i].alerted = true;
            isVibrating = true;
            motorStartMillis = millis();

            // ========= INICIA NOVA MUSIQUINHA =========
            melodyPlaying = true;
            currentNoteIndex = 0;
            isSilentPeriod = false;
            noteStartMillis = millis();
            
            // Calcula a duração da primeira nota baseado no primeiro código
            noteTotalDuration = duration[currentNoteIndex] * tempo; 

            if (notes[currentNoteIndex] != ' ') {
                int freq = getToneFrequency(notes[currentNoteIndex]);
                if (freq > 0) tone(PIEZO, freq, noteTotalDuration);
            }
            // ==========================================

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("TOME:");

            digitalWrite(GREEN_LED, HIGH);

            lcd.setCursor(0, 1);
            lcd.print(medications[i].name);
            break;
        }
    }

    // ========= CONTROLA NOVA MUSIQUINHA (MÁQUINA DE ESTADOS NÃO-BLOQUEANTE) =========
    if (melodyPlaying) {
        unsigned long elapsed = millis() - noteStartMillis;

        if (!isSilentPeriod) {
            // Se o tempo da nota atual acabou, entra no período de silêncio (pausa entre notas)
            if (elapsed >= noteTotalDuration) {
                noTone(PIEZO);
                isSilentPeriod = true;
                noteStartMillis = millis();
                // O primeiro código usava delay((tempo*2)*duration[i]) como pausa total após tocar.
                // Para manter proporcional, definimos esse tempo de pausa aqui:
                noteTotalDuration = (tempo * 2) * duration[currentNoteIndex];
            }
        } else {
            // Se o período de silêncio acabou, passa para a próxima nota
            if (elapsed >= noteTotalDuration) {
                currentNoteIndex++;
                isSilentPeriod = false;
                noteStartMillis = millis();

                if (currentNoteIndex < totalNotes) {
                    noteTotalDuration = duration[currentNoteIndex] * tempo;
                    if (notes[currentNoteIndex] != ' ') {
                        int freq = getToneFrequency(notes[currentNoteIndex]);
                        if (freq > 0) tone(PIEZO, freq, noteTotalDuration);
                    }
                } else {
                    // Fim da música
                    noTone(PIEZO);
                    melodyPlaying = false;
                }
            }
        }
    }
    // ===============================================================================

    if (isVibrating && (millis() - motorStartMillis >= VIBRATION_DURATION_MS)) {
        digitalWrite(GREEN_LED, LOW);
        isVibrating = false;

        // Garante que o som pare caso a música ainda esteja tocando ao encerrar o alarme
        noTone(PIEZO);
        melodyPlaying = false;

        for (int i = 0; i < totalMedications; i++) {
            if (medications[i].hour == currentHour && medications[i].alerted) {
                removeMedication(i);
                break;
            }
        }
    }
}

void updateClock() {
    if (totalMedications == 0) {
        previousMillis = millis();
        return;
    }

    if (millis() - previousMillis >= SIMULATED_HOUR_MS) {
        previousMillis += SIMULATED_HOUR_MS;
        currentHour++;

        if (currentHour > 23) {
            currentHour = 0;
            for (int i = 0; i < totalMedications; i++) {
                medications[i].alerted = false;
            }
        }
        forceScreenUpdate = true;
    }
}

void loop() {
    if (Serial.available() > 0 && !isVibrating) {
        registerMedication();
    }

    updateClock();
    handleAlertsAndMotor();
    displayNextMedication();
}