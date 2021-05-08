# Where-is-my-cat

  Bem-vindo ao projeto Where is my cat, cujo o objetivo é praticar o uso de diversas tecnologias em um projeto hobbysta.

  Se possuir interesse em participar desse pequeno projeto, deixe sua contribuição. Caso deseje, você pode entrar em contato comigo para esclarecer dúvidas ou informar sobre contribuições a serem feitas, meu contato principal é o meu e-mail: 
hilariojscastro@gmail.com .

  Como o nome do projeto sugere, "Encontro meu gato", tem como ideia encontrar o meu gato (chamado Black) em minha residência. Atualmente eu moro em um apartamento e o gato não consegue sair da região do apartamento, delimitando a região e facilitando a visualização e forma de encontrar nosso alvo. 

  O projeto atualmente está dividido em 3 partes, onde a primeira parte é um sistema de hardware que estou trabalhando, que  será a coleira que possuirá um sistema embarcado capaz de realizar comunicação via Wi-Fi. A coleira irá coletar amostras das redes Wi-Fi alcançáveis e irá transmitir as amostras registradas via um tópico (WMC/Beacon) do protocolo de comunicação MQTT (estou utilizando este broker online: http://www.hivemq.com/demos/websocket-client/?). Atualmente a coleta dos dados dos arquivos de registros das pastas do projeto foram feitas por um ESP32 em uma protoboard parada nos lugares favoritos do gato em cada ambiente da residência.

  A segunda parte do projeto é um pequeno servidor que estou montando para servir de interação da coleira com outros sistemas. Atualmente este servidor atua apenas como um coletador de dados e está com a versão que utilizei para registrar os dados das redes próximas em arquivos json e csv. A tendência será que eu torne esse servidor apenas na opção de registrar valores das redes coletadas pela coleira e informar os últimos valores coletados via MQTT para outros sistemas, bem como realizar possíveis novas solicitações.

  A terceira parte é relacionado a criação do aplicativo para apresentar onde está o gato (Black) na minha residência.  Atualmente esta parte está apenas com uma avaliação de algoritmos de machine learning para um ser escolhido para encontrar o gato com novos valores e visualizar os dados da versão atual dos registros das redes. É possível conferir essas análises inicial nos seguintes links:
* Visualização dos dados - https://drive.google.com/drive/folders/1k6korMrMqeRkQRh_3asx2qyI8VEUgpi3?usp=sharing
* Avaliação em algoritmos para localização - https://drive.google.com/drive/folders/1yUITgPPDRj7WqIRdF0h0cTYHJF87L524?usp=sharing

O projeto de localização indoor (Indoor Location) atualmente está proposto por utilizar uma solução baseada em redes do tipo de alcance livre, utilizando como concepção inicial o uso da técnica Fingerprint, mas existe a previsão de tentar outras abordagens. 

Os próximos trabalhos que estarei fazendo e atualizando no projeto são:
* Prototipação da coleira para coletas reais;
* Criação da planta do apartamento, em vários formatos de arquivo;
* Estudar acrescentar uso de uma IMU (possui acelerômetro, giroscópio e magnetômetro) no sistema da coleira e 
como utilizar os novos dados;
* Criar aplicativo para smartphone que visualize a posição atual do dado e dados coletados.

Uma opinião minha é ajudar nos métodos de localização e criação do aplicativo de visualização.
