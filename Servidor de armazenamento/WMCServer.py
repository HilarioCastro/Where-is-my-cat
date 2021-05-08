'''Import das bibliotecas, caso exista alguma que não esteja instalada e não seja de 
desenvolvimento desde projeto será feito a instalação via pip e feito uma nova tentativa depois'''
import sys
import json
from datetime import datetime

import paho.mqtt.client as mqtt 


#Declaração de funções

# Iniciando algumas variáveis como globais
def setDadosIniciais(con):
    global CONFIG

    CONFIG = con


    return CONFIG


def formatar_e_Salvar_Redes(DATA):
    #Função para formatar os dados recebidos via MQTT
    #A mensagem chegará em dois possíveis formatos, como mostra a seguir:
    #
    #PrimeiroFormato
    #"SSIDRede01;leitura1;leitura2;leitura3;leitura4;leitura5/SSIDRede02;leitura1;leitura2;leitura3;leitura4;leitura5/...
    #
    #Segundo formato
    #"SSIDRede01;leitura/SSIDRede02;leitura/..."
    #
    #Isso é, ou virá apenas uma leitura de cada rede ou 5 leituras de cada rede. No caso de várias leituras 
    #será feita a média das amostras para formar um valor resultante, caso contrário, será trabalhado apenas um
    #valor como amostra.
    #
    #De toda forma, serão duas mensagens com conteúdo de todas as redes, assim sendo salvo num arquivo em formato
    #json como proposta de trabalho. De forma extra, também esta sendo salvo os dados como csv com dados
    # separados por ";". 
    #try:

        local = "Quarto03"

        REDES_E_RSSI={}
        CONJUNTO_DE_REDES= DATA.split('/') #Quebra/parte a string em parte baseado em um caractere/string de referência, no caso a '/'
                            #retorna uma list como no exemplo: ["SSIDRede01;leitura1;leitura2;leitura3;leitura4;leitura5", "SSIDRede02;leitura1;leitura2;leitura3;leitura4;leitura5]
        #Para registrar horário de recepção do dado
        HORARIO_ATUAL = datetime.now()
        HORARIO = HORARIO_ATUAL.strftime("%d/%m/%Y %H:%M:%S")

        for REDE in CONJUNTO_DE_REDES:

            LISTA_DE_VALORES_DE_UMA_REDE = REDE.split(';') # Retorna uma lista como no exemplo: ["SSIDRede01", "leitura1", "leitura2", "leitura3", "leitura4" "leitura5"]
            RSSI=0
            DIV = 0
            for VALOR_REDE in LISTA_DE_VALORES_DE_UMA_REDE[1:]:
                RSSI+=int(VALOR_REDE)
                if int(VALOR_REDE) != 0:
                    DIV+=1
            if DIV == 0:
                DIV = 1
            #posição 0 da lista é sempre o nome da rede, demais valores são as RSSI coletadas
            REDES_E_RSSI[LISTA_DE_VALORES_DE_UMA_REDE[0]] = [HORARIO, RSSI/DIV] 

        if(local !=''):
            REDES_E_RSSI["LocalAtual"] = [HORARIO, local]


        #Primeiro formato de salvas os dados será um arquivo json no formato: 
        #                                           {"Nome de rede": [Horário da mensagem, RSSI]}
        #o segundo formato são os dados em um arquivo .csv separados por ";"

        #Sempre que a mensagem recebida não seja a última mensagem do pacote, os dados dos pacotes
        #das redes anteriores serão atualizados em um arquivo a parte para sere, anexados com o último
        #pacote no final no arquivo definitivo.


        #primeiro formato, arquivo json
        try:

            obj2 = open('Registros_de_redes_WMC.json', 'r', encoding="utf8")
            SALVAR_NOVOS_REGISTROS_DE_REDES = json.load(obj2)
            obj2.close()

        except:

            with open('Registros_de_redes_WMC.json','w', encoding="utf8") as fd:
                fd.write('')
            SALVAR_NOVOS_REGISTROS_DE_REDES ={}

        for REDE in REDES_E_RSSI:
            if REDE not in SALVAR_NOVOS_REGISTROS_DE_REDES:
                temp={}
                temp[REDE]=[REDES_E_RSSI[REDE]]
                SALVAR_NOVOS_REGISTROS_DE_REDES.update(temp)
   
            else:
                SALVAR_NOVOS_REGISTROS_DE_REDES[REDE].append(REDES_E_RSSI[REDE])
    

        salvar_CSV = json.dumps(SALVAR_NOVOS_REGISTROS_DE_REDES, indent=4)
        with open('Registros_de_redes_WMC.json','w', encoding="utf8") as fd:
            fd.write(salvar_CSV)
 

        #Formato csv separado por ';'

        #Preparando a mensagem no formato -> Nome da rede;RSSI;
        SALVAR_REDES=HORARIO
        for REDE in REDES_E_RSSI:
            if(REDE != "LocalAtual"):
                SALVAR_REDES+=REDE+';'+str(REDES_E_RSSI[REDE][1])+';'

        if(local !=''):
            SALVAR_REDES+=local+";"

 
        #lendo as informações do arquivo temporário e em seguida, adicionar a nova leitura
        '''try:
            obj = open('Registros_de_redes_WMC.csv','r', encoding="utf8")
            SALVAR_NOVAS_REDES_CSV = obj.read()
            obj.close()

        except:
            #caso o arquivo não exista, cria o arquivo e inicia seu valor como um texto
            with open('Registros_de_redes_WMC.csv','w', encoding="utf8") as fd:
                fd.write('')'''

        '''if(SALVAR_NOVAS_REDES_CSV !=''):
            SALVAR_NOVAS_REDES_CSV= SALVAR_NOVAS_REDES_CSV+SALVAR_REDES
        else:
             SALVAR_NOVAS_REDES_CSV=SALVAR_REDES'''

        with open('Registros_de_redes_WMC.csv','a', encoding="utf8") as fd:
                fd.write(SALVAR_REDES)
        

    #except:
    #    print('Erro no formato da mensagem')

# Publica no broker mqtt a mensagem enviada no tópico enviado para ela.
def mqtt_publish(mensagem, topico_mqtt, config_mqtt):
    client = mqtt.Client()

    client.connect("broker.mqttdashboard.com", 1883, 60)
    #client.connect(config_mqtt['ip'], config_mqtt['port'])

    if(topico_mqtt == CONFIG["mqtt_topicos_pub"]["atualiza"]):
            print("Enviando para o broker MQTT no topico {} o seguinte dado: {}".format(topico_mqtt, mensagem))
            client.publish(topico_mqtt, mensagem, qos=0, retain=False)


    return "MQTT Publish OK"


# "Thread" do mqtt que ficará escutando novas atualizações nos tópicos escolhidos.
def on_message(client, userdata, msg):
    #try:
        print("\nTópico Recebido:", msg.topic + " \nCom a respectiva mensagem: " + msg.payload.decode('utf-8'))

        if((msg.topic in CONFIG['mqtt_topicos_sub']['Beacon'])):
            formatar_e_Salvar_Redes(msg.payload.decode('utf-8'))
    
        
        elif((msg.topic in CONFIG['mqtt_topicos_sub']['Reply'])):
            print("Precisa criar função do reply")

    
        elif((msg.topic in CONFIG['mqtt_topicos_sub']['Dataset'])):
            print('precisa criar função para consulta de dataset')

    #except:
    #    print('Erro no formato da mensagem.')

    
def config_mqtt(mqtt_config):
    # Configurar a conexão com o broker mqtt, 
    # retornando o objeto "client" para que possa executar o comando de stop, caso seja necessário
    #client = mqtt.Client(client_id="AT", transport = "websockets")
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect("broker.mqttdashboard.com", 1883, 60)
    #client.connect("broker.mqttdashboard.com", 8000, 60)
    #client.connect(mqtt_config["ip"], mqtt_config["port"], mqtt_config["keep_alive"])

    return client


def on_connect(client, userdata, flags, rc):
    # Função para se inscrever em todos os tópicos enviados.
    #Conectando em todos os tópicos conhecidos, que são gerados a partir dos id dos satelites conhecidos
    MQTT_TOPICOS_SUB = []
    MQTT_TOPICOS_SUB = CONFIG["mqtt_topicos_sub"]
    print('connected with result code', str(rc))
    if MQTT_TOPICOS_SUB:
        if not rc:
            print("conectado ao broker MQTT.")
            #print(mqtt_topicos_sub)
            if (type(MQTT_TOPICOS_SUB) == list):
                for TOPICO in MQTT_TOPICOS_SUB:
                    print("conectado ao tópico:", TOPICO)
                    client.subscribe(TOPICO)
                    pass
            elif(type(MQTT_TOPICOS_SUB) == dict):
                for TOPICO in MQTT_TOPICOS_SUB:
                    print("conectado ao tópico:", CONFIG['mqtt_topicos_sub'][TOPICO])
                    client.subscribe(CONFIG['mqtt_topicos_sub'][TOPICO])
            else:
                print("conectado ao tópico:", MQTT_TOPICOS_SUB)
                client.subscribe(MQTT_TOPICOS_SUB)
            #escrevendo no topic savesatid
            #client.subscribe(config["mqtt_topicos_sub"]["savesatid"])
        else:
            print("Erro na conexão com o broker MQTT.")
    else:
        print("\nEncerrando código WMC...")
        sys.exit()



'''
Carregando dados gerais
'''

obj = open('WMCConfig.json', 'r')
CONFIG = json.load(obj2)
obj.close()

# Pegando os dados sobre o broker mqtt do arquivo de configuração geral
BROKER_CONFIG = CONFIG['broker_mqtt']


'''
Set do arquivo de configuração como variável global
'''
CONFIG = setDadosIniciais(CONFIG)    

#incializando o client mqtt
CLIENT_MQTT = config_mqtt(BROKER_CONFIG)
#deixando o client MQTT como base do sistema rodar sempre


CLIENT_MQTT.loop_forever()