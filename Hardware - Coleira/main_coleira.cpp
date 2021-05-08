/* Headers*/
#include <typeinfo>
#include <Arduino.h>
#include <WiFi.h> // Header para uso das funcionalidades de wi-fi do ESP32 
#include <PubSubClient.h>  //  Header para uso da biblioteca PubSubClient (versão de https://github.com/knolleary/pubsubclient)
#include <String.h>// Header para uso da biblioteca Strings
//#include <NTPClient.h> // Header para uso da Bibliotecas para o NTP Server (versão de: https://github.com/taranais/NTPClient)

/*########################################  MQTT  ####################################################*/
/*########################################  MQTT  ####################################################*/
/*########################################  MQTT  ####################################################*/
/* DEFINES do #MQTT */
/* Tópico MQTT para recepção de informações do broker MQTT para ESP32 */
#define TOPICO_SUBSCRIBE "WMC/Request"   
/* Tópico MQTT para envio de informações do ESP32 para broker MQTT */
#define TOPICO_PUBLISH   "WMC/Beacon"  
/*O ID deve ser único, caso outro dispositivo tenha o mesmo ID esse será desconectado quando o 
segundo se conectar 
 \/ \/ \/ \/ \/ \/ \/ \/ \/   */
#define ID_MQTT  "Coloque_UM_ID_MQTT"     
/* URL do broker MQTT que deseja utilizar */
const char* BROKER_MQTT = "broker.hivemq.com"; 
/* Porta do Broker MQTT   
Recomendo manter esse valor (1883) como padrão, mas para mensagens criptografadas é utilizada outra
porta. Mais detalhes conferir na documentação da biblioteca.
*/
int BROKER_PORT = 1883;

/*#########################################  WIFI  ###################################################*/
/*#########################################  WIFI  ###################################################*/
/*#########################################  WIFI  ###################################################*/
/* DEFINES, Variáveis e constantes globais para o #Wifi*/
/* SSID (nome da rede) WI-FI que deseja se conectar */
const char* SSID = "Coloque o nome da sua rede aqui"; 
/* PASSWORD (Senha) da rede WI-FI que deseja se conectar */
const char* PASSWORD = "senha da sua rede aqui"; 
   
/*#########################################  Prototypes  ###################################################*/
/*#########################################  Prototypes  ###################################################*/
/*#########################################  Prototypes  ###################################################*/
/* Variáveis e objetos globais */
WiFiClient espClient;
PubSubClient MQTT(espClient);
//Variáveis auxiliares para controlar a passagem de tempo
unsigned int AUXILIAR_DE_TEMPO = 0;
unsigned int AUXILIAR_DE_TEMPO_2 = 0;
int TEMPO_DE_AMOSTRAGEM = 60000; // 1 minuto
int NRC = 11; //Número de Redes conhecidas
String REDES_CONHECIDAS[] = {}; // Esse array é utilizado junto com o da linha a seguir para pegar os nomes das redes conhecidas e converter para outro nome 
String NOMES_DAS_REDES[] ={"Rede01","Rede02","Rede03","Rede04","Rede05","Rede06","Rede07","Rede08","Rede09","Rede10","Rede11"};
int REDEAUX = 0; //Variável auxiliar para ajudar a contar amostras e fazer médias
int AMOSTRAS_DE_LEITURAS = 20; //Variável auxiliar para definir a cada quantos segundos será feita uma amostra
int AMOSTRAS = 5;
bool AUX_de_ATUALIZACAO = false;
String NOMES_DE_REDES_CONHECIDAS[13];
int RSSI_DE_REDES_CONHECIDAS[13][5];
//Para o WatchDog
hw_timer_t *TIMER = NULL; //faz o controle do temporizador (interrupção por tempo)

//Prototypes das funções utilizadas
void init_Serial(void); //Para Comunicação Serial do ESP32
void init_Wifi(void); //Iniciar Conexão com a rede Wifi 
void init_MQTT(void); //Iniciar Conexão com o broker MQTT
void reconnect_Wifi(void); //Utilizado para conectar e reconectar com a rede wifi
void mqtt_Callback(char* topic, byte* payload, unsigned int length); //para receber mensagens MQTT
void verifica_Conexoes_Wifi_Mqtt(void);//Conferindo conexões  MQTT
void init_WatchDog(void); // Inicializa o Watchdog
void resetModule(void); // Caso o watchdog seja ativado, irá ser chamado para incializar o ESP32
void confereWatchDog(void); //Função para atualizar o WatchDog e não deixar este incializar o ESP32
bool RegistraRSSIRedes(void); //Função para coletar as redes conhecidas
String mensagemParaPublicacao(void); // Preparar a mensagem para a publicação


/*#########################################  Implementações das funções  ###########################################*/
/*#########################################  Implementações das funções  ###########################################*/
/*#########################################  Implementações das funções  ###########################################*/


/* Função: inicializa comunicação serial com baudrate 115200 (Padrão do ESP32).
* Parâmetros: nenhum
* Return: nenhum
*/
void init_Serial() 
{
    Serial.begin(115200);
}
 
/* Função: inicializa, conecta e reconecta o ESP32 na rede WI-FI desejada
 * Parâmetros: nenhum (Variáveis SSID e PASSWORD estão globais)
 * Return: nenhum
 */
void init_Wifi(void) 
{
    delay(10);
    Serial.println("Iniciando Conexao WI-FI com a Rede...");
    Serial.println(SSID);
    Serial.println("Aguardando conexão...");
    reconnect_Wifi();
}
  
/* Função: inicialização da conexão MQTT
 * métodos MQTT chamados MQTT: endereço do broker (variável Global BROKER_MQTT);
 *                             porta MQTT ((variável Global BROKER_PORT));
 *                             set função de callback;
 * Parâmetros: nenhum (tudo que precisa esta global)
 * Return: nenhum (Tudo esta setado como global)
 */
void init_MQTT(void) 
{
    /* set de conexão com o broker MQTT e sua PORT */
    MQTT.setServer(BROKER_MQTT, BROKER_PORT); 
    /* atribui função de callback (função chamada quando qualquer informação do 
    *                              tópico subescrito chega) 
    */
    MQTT.setCallback(mqtt_Callback);            
}
  
/* Função: função de callback 
 *          esta função é chamada toda vez que uma informação de 
 *          um dos tópicos subescritos chega)
 * Parâmetros: nenhum
 * Retorno: nenhum
 * */
void mqtt_Callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
 
    //Recebe a string do payload recebido em tópico inscrito (subscribed) MQTT
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
    Serial.print("Tópico atualizado: ");
    Serial.println(topic);
    Serial.print("Com a mensagem: ");
    Serial.println(msg);     
}
  
/* Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
 *          em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void reconnect_mqtt(void) 
{   
/*Caso a conexão com o tópico MQTT já estiver feita, simplesmente não faz nada
*Caso contrário, é inicializado as tentativas de conexão com o broker e o sistema ficará preso
*(blockado) até que a conexão seja concluída.
*/
    while (!MQTT.connected()) 
    {
    /*Primeiro passo será tentar a incialização da conexão pela chamada do método connect.
    *Em seguida será feita a inscrição no tópico utilizando o método subscribe.
    *Cada método requer seus atributos (Em caso de dúvidas, conferir documentação da biblioteca).
    */    
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        Serial.print("...");
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.print("Conectado com sucesso ao broker MQTT ");
            Serial.print(BROKER_MQTT);
            Serial.println("!");
            Serial.println("Fazendo inscrição (Subscribe) no tópico: ");
            Serial.println(TOPICO_SUBSCRIBE);
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker. Tentando novamente em 1 segundo...");
            delay(1000);
        }
    }
}
  
/* Função: reconecta-se ao WiFi
 * Parâmetros: nenhum
 * Retorno: nenhum
*/
void reconnect_Wifi() 
{
    /* se já está conectado a rede WI-FI, nada é feito. */
    if (WiFi.status() == WL_CONNECTED)
        return;
    /*Caso contrário, são efetuadas tentativas de conexão */
    
    /*Primeiro são passados os dados de SSID e PASSWORD para tentar inicialização. */
    WiFi.begin(SSID, PASSWORD);

    /*Criado um laço onde deixa o sistema preso (blockado) até que a conexão seja realizada*/ 
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
 
/* Função: verifica o estado das conexões WiFI e ao broker MQTT. 
 *         Em caso de desconexão (qualquer uma das duas), a conexão
 *         é refeita.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void verifica_Conexoes_Wifi_Mqtt(void)
{
    /* se não há conexão com o WiFI, a conexão é refeita */
    reconnect_Wifi(); 
    /* se não há conexão com o Broker, a conexão é refeita */
    if (!MQTT.connected()) 
        reconnect_mqtt(); 
} 

/*Pode acontecer que o sistema trave por algum motivo, portanto será feito um watchdog para garantir
reinicializar o ESP32 em caso de algum problema*/ 
void init_WatchDog(void)
{
  //Configurações para o WatchDog e suas interrupções
  //hw_timer_t * timerBegin(uint8_t num, uint16_t divider, bool countUp)
  /*
    num: é a ordem do temporizador, o sistema tem quatro temporizadores com registros [0,1,2,3].

    divider: É um prescaler (ajuste de escala, no caso da frequencia por fator).
    Para fazer um agendador de um segundo será utilizado o divider como 80 (clock principal do ESP32 é 80MHz). Cada
    instante será T = 1/(80) = 1us

    countUp: True o contador será progressivo
  */
  TIMER = timerBegin(0, 80, true); //timerID 0, div 80
  //TIMER, callback, interrupção de borda
  timerAttachInterrupt(TIMER, &resetModule, true);
  //TIMER, tempo (us), repetição
  timerAlarmWrite(TIMER, 60000000, true);
  timerAlarmEnable(TIMER); //habilita a interrupção
}


//função que o temporizador irá chamar, para reiniciar o ESP32
void IRAM_ATTR resetModule() {
  ets_printf("(watchdog) reiniciar\n"); //imprime no log
  esp_restart(); //reinicia o chip
}

//Função para conferir se o Esp32 travou
void confereWatchDog() {
  timerWrite(TIMER, 0); //reseta o temporizador (alimenta o watchdog)
}  

/*Função para coletar o RSSI das redes conhecidas*/
bool RegistraRSSIRedes() {
 
    //Realizando o Scan das redes
    int SCAN_REDES = WiFi.scanNetworks();
    //Conferindo se as redes encontradas pertencem ao conjunto de redes conhecidas
    for (int NUMERO_DA_REDE_ENCONTRADA = 0; NUMERO_DA_REDE_ENCONTRADA < SCAN_REDES; NUMERO_DA_REDE_ENCONTRADA++) {

        for (int NUMERO_DA_REDE_CONHECIDA = 0; NUMERO_DA_REDE_CONHECIDA < NRC; NUMERO_DA_REDE_CONHECIDA++) {
            if (WiFi.SSID(NUMERO_DA_REDE_ENCONTRADA) == REDES_CONHECIDAS[NUMERO_DA_REDE_CONHECIDA]) { //Caso sejam conhecidas, as redes
                if( WiFi.SSID(NUMERO_DA_REDE_ENCONTRADA) == "HilAna_2GHz"){

                }
                //NOMES_DE_REDES_CONHECIDAS[NUMERO_DA_REDE_CONHECIDA] = NOMES_DAS_REDES[NUMERO_DA_REDE_CONHECIDA];            //NOMES_DE_REDES_CONHECIDAS[w] = WiFi.SSID(x);          //serão armazenadas nas respectivas
                RSSI_DE_REDES_CONHECIDAS[NUMERO_DA_REDE_CONHECIDA][REDEAUX] = WiFi.RSSI(NUMERO_DA_REDE_ENCONTRADA); //posições dos arrays auxiliares
                delay(10);
  
            }
        }
    }
    /*A cada ciclo de contagem, é feito um registro de amostra, no intuito de formar as 5 amostras desejadas
    //de cada rede conhecida. As amostras são controladas por meio da variável auxiliar "REDEAUX", que quando
    //atingir o limite configurado, irá sinalizar a atualização, como sendo um return um boolean 
    (true para atualizar e false para não).
    */


    REDEAUX=REDEAUX+1;
    //Caso seja feito os 5 registros, retorna true
    if(REDEAUX >=AMOSTRAS){
        return true;
    }
    //Caso contrário retorna false, indicando que não precisa ser feita uma atualização
    return false;
    }


/*Esta Função tem como objetivo preparar a mensagem no formato desejado
*O Formato a ser trabalhado seráa de uma string com algns parâmetros concatenados e separados por
*caracteres especificos.
*Portanto, o padrão estipulado será: 
*"SSIDRede01;leitura1;leitura2;leitura3;leitura4;leitura5/SSIDRede02;leitura1;leitura2;leitura3;leitura4;leitura5/...
*/
String mensagemParaPublicacao(){
      String MENSAGEM_COM_REDES = "";
      for(int i = 0; i < NRC; i++){
          //Serial.print(NOMES_DE_REDES_CONHECIDAS[i]);
          MENSAGEM_COM_REDES+=NOMES_DAS_REDES[i]+";";
         // Serial.print("Conferindo cada caractere: ");
          for(int x = 0; x< AMOSTRAS; x++){
            //Serial.println(String(RSSI_DE_REDES_CONHECIDAS[i][x]));
              MENSAGEM_COM_REDES+=String(RSSI_DE_REDES_CONHECIDAS[i][x]);
              if(x != AMOSTRAS-1){
                  MENSAGEM_COM_REDES+=';';
              }
          //Serial.println("Conferindo mensagem - ");    
          //Serial.print(MENSAGEM_COM_REDES);
          }
          if(i != NRC-1){
          MENSAGEM_COM_REDES+="/";
          }
      }
      MENSAGEM_COM_REDES = MENSAGEM_COM_REDES;
      Serial.println(MENSAGEM_COM_REDES);
      return MENSAGEM_COM_REDES;
  }


void zerarRegistros(){
    for(int x =0; x<NRC;x++){
      //NOMES_DE_REDES_CONHECIDAS[x]=NOMES_DAS_REDES[x];
      for(int i=0;i<5;i++){
        RSSI_DE_REDES_CONHECIDAS[x][i] =0;
      }
    }
    REDEAUX=0;
}


/*#########################################  LOOP principal  ###########################################*/
/*#########################################  LOOP principal  ###########################################*/
/*#########################################  LOOP principal  ###########################################*/

/*Nesse ponto deverá seguir a idealização da chamada das funções de acordo com o comportamento desejado
*para a aplicação. 
*Como na aplicação do "Onde esta meu gato?" (Where's my cat?) é a idealização de uma coleira portando
*o ESP32 então este estará se deslocando no ambiente, o que significa que o sistema poderá perder a 
*conexão com o restante do sistema porque o gato foi para algum lugar com sinal fraco ou ausente.
*Então é interessante sempre ficar conferindo a conexão a rede Wifi e com tópico MQTT, para o sistema
*perca conexão seja inicializado uma nova conexão.
*
*O sistema se baseia na coleta de amostras das intensidades das redes conhecidas e sinaliza para o
*restante da rede via tópico MQTT. Os valores coletados são enviados como uma string em um formato 
*específico, sendo este formato:
*
*"SSIDRede01;leitura1,leitura2,leitura3,leitura4,leitura5/SSIDRede02;leitura1,leitura2,leitura3,leitura4,leitura5/..." 
*
*Como pode ser notado no formato especificado, serão feitas 5 amostras das intensidades dos sinais
das redes, sendo estas 5 amostras mais recentes das mesmas. Caso alguma rede não possua amostra, essa
*rede terá sua amostra com valor zero, essa rede será descartada. Outro ponto a lembrar, que serão
* enviados apenas das redes conhecidas e registradas.
*
*O Envio desses dados é feito apenas quando o sistema receber uma nova postagem em tópico MQTT, no 
*qual o sistema esta inscrito. Ao ser sinalizada a requisição, o ESP32 irá postar em seu tópico os
*valores coletados.
*/
void setup() 
{
    /*Durante o setup é inicializado as funções e definições de pinos de entrada e saída*/
    init_Serial();
    init_WatchDog();
    init_Wifi();
    init_MQTT();
    /*Primeiro registro de passagem de tempo -> antes de entrar no loop principal*/
    AUXILIAR_DE_TEMPO = millis();
    AUXILIAR_DE_TEMPO_2 = millis();
    //Inicializando o nome das redes
    for(int x =0; x< NRC; x++){
      NOMES_DE_REDES_CONHECIDAS[x] = NOMES_DAS_REDES[x];
      
      }
    zerarRegistros();
}
  

void loop() 
{   
    /* garante funcionamento das conexões WiFi e ao broker MQTT */
    verifica_Conexoes_Wifi_Mqtt();

    /* Envia frase ao broker MQTT */
    if(AUX_de_ATUALIZACAO){
        Serial.println("ESP32 se comunicando com MQTT para publicar resultados");
        //Primeira parte dos satélites
        String ms = mensagemParaPublicacao();
        /*Convertendo de String para array de chars*/
        int SCAN_REDES = ms.length();
        char char_array[SCAN_REDES + 1];
        strcpy(char_array, ms.c_str());
        MQTT.publish(TOPICO_PUBLISH, char_array);
        /*ms = mensagemParaPublicacao(6, 0, 1);
        SCAN_REDES = ms.length();
        char_array[SCAN_REDES + 1];
        strcpy(char_array, ms.c_str());
        MQTT.publish(TOPICO_PUBLISH, char_array);*/

        zerarRegistros();
        AUX_de_ATUALIZACAO = false; //para sair logo em seguida e não repetir postagens no tópico MQTT
 
    }
    /* keep-alive da comunicação com broker MQTT */    
    MQTT.loop();

    //A cada passagem de tempo será feita uma amostragem da leitura dos sensores
    if (millis() - AUXILIAR_DE_TEMPO > (TEMPO_DE_AMOSTRAGEM / AMOSTRAS_DE_LEITURAS)) {
        AUX_de_ATUALIZACAO = RegistraRSSIRedes();
        //Atualiza AUXILIAR_DE_TEMPO
        AUXILIAR_DE_TEMPO = millis();
    }

    /*Função para atualizar o watchdog e conferir se o ESP32 travou muito tempo por algum motivo
    *Caso o dispositivo tenha travado, ele irá reiniciar sozinho
    Situações comuns de reinicializar: Perder conexão e passar muito tempo blockado nas tentativas
    */
    confereWatchDog(); //60 segundos sem ser atualizado vai reiniciar o ESP32   

    /* Aguarda 1 segundo para próximo loop */
    delay(1000);
}