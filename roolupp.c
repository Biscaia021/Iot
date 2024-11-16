#include <Stepper.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pinos
#define RST_PIN         9         // Pino de reset do MFRC522
#define SS_PIN          10        // Pino do SS (Slave Select) do MFRC522
#define endereco        0x27      // Endereço do LCD I2C
#define colunas         16        // Número de colunas do LCD
#define linhas          2         // Número de linhas do LCD

// Definição dos pinos do motor de passo
int STEPPER_PIN_1 = 6;
int STEPPER_PIN_2 = 7;
int STEPPER_PIN_3 = 3;
int STEPPER_PIN_4 = 2;

// Inicialização do LCD
LiquidCrystal_I2C lcd(endereco, colunas, linhas);

bool Locked = false;    // Variável para controlar o estado de travamento da porta
const float Steps = 11.38;   // Número de passos por rotação do motor de passo

String IDtag = "";      // Variável que armazenará o ID da Tag RFID lida
bool Permitido = false; // Variável que verifica se o acesso é permitido

// Vetor responsável por armazenar os IDs das Tags cadastradas
String TagsCadastradas[] = {"35c5719"};

// Criação de uma instância do leitor RFID MFRC522
MFRC522 LeitorRFID(SS_PIN, RST_PIN);

// Criação de uma instância do motor de passo
Stepper mp(Steps, STEPPER_PIN_1, STEPPER_PIN_3, STEPPER_PIN_2, STEPPER_PIN_4);

void setup() {
    Serial.begin(9600);        // Inicializa a comunicação Serial
    SPI.begin();               // Inicializa a comunicação SPI com o MFRC522
    LeitorRFID.PCD_Init();     // Inicializa o leitor RFID
    
    // Inicialização do LCD
    lcd.init(); 
    lcd.backlight();
    lcd.clear(); 
    lcd.print("- Ola, Mundo! -"); // Exibe uma mensagem inicial no LCD
    delay(5000); 
    lcd.setCursor(0, 1); 
    lcd.print("Fim do Setup ()");
    delay(5000); 
    lcd.noBacklight();    // Desliga o backlight do LCD temporariamente
    delay(2000);
    lcd.backlight();      // Liga o backlight do LCD
    delay(2000);
    lcd.clear();          // Limpa o LCD
    
    mp.setSpeed(500);     // Define a velocidade do motor de passo

    Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
    // Exibe uma mensagem inicial no monitor Serial
}

void UnlockDoor() {
    for (float i = 0; i < 420; i++) {
        mp.step(-Steps);   // Realiza passos no sentido de destrancar a porta
    }
    Locked = false;       // Atualiza o estado da porta para destrancar
    lcd.clear();
    lcd.print("Porta Trancada"); // Exibe mensagem de porta destrancar no LCD
}

void LockDoor() {
    for (float i = 0; i < 590; i++) {
        mp.step(Steps);  // Realiza passos no sentido de trancar a porta
    }
    Locked = true;        // Atualiza o estado da porta para trancar
    lcd.clear();
    lcd.print("Porta Destrancada"); // Exibe mensagem de porta trancada no LCD
}

void loop() {  
    Leitura();  // Chama a função responsável por fazer a leitura das Tags RFID
}

void Leitura() {
    IDtag = ""; // Limpa o valor anterior do IDtag
    
    // Verifica se existe uma Tag presente
    if (!LeitorRFID.PICC_IsNewCardPresent() || !LeitorRFID.PICC_ReadCardSerial()) {
        delay(50);  // Aguarda um curto período de tempo
        return;
    }
    
    // Lê o ID da Tag RFID e armazena na variável IDtag
    for (byte i = 0; i < LeitorRFID.uid.size; i++) {        
        IDtag.concat(String(LeitorRFID.uid.uidByte[i], HEX));
    }
    
    // Compara o valor do ID lido com os IDs armazenados no vetor TagsCadastradas[]
    for (int i = 0; i < (sizeof(TagsCadastradas) / sizeof(String)); i++) {
        if (IDtag.equalsIgnoreCase(TagsCadastradas[i])) {
            Permitido = true; // Define que o acesso é permitido se o ID estiver cadastrado
            break;
        }
    }
    
    // Verifica se o acesso é permitido e executa ações correspondentes
    if (Permitido) {
        acessoLiberado(); // Chama a função acessoLiberado() se o acesso for permitido
    } else {
        acessoNegado();   // Chama a função acessoNegado() se o acesso não for permitido
    }
    
    delay(2000); // Aguarda 2 segundos antes de fazer uma nova leitura
}

void acessoLiberado() {
    Serial.println("Tag Cadastrada: " + IDtag); // Exibe mensagem de tag cadastrada no monitor Serial
    Permitido = false;  // Reseta a variável Permitido para próxima leitura
    
    // Verifica o estado da porta e executa a função correspondente
    if (Locked) {
        UnlockDoor(); // Destrava a porta se estiver trancada
    } else {
        LockDoor();   // Trava a porta se estiver destrancada
    }
}

void acessoNegado() {
    Serial.println("Tag NAO Cadastrada: " + IDtag); // Exibe mensagem de tag não cadastrada no monitor Serial
    lcd.clear();
    lcd.print("Acesso Negado"); // Exibe mensagem de acesso negado no LCD
}