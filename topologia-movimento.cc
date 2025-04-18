//Aqui está outro código utilizado no simulador de redes NS-3
//Esse código contém 5 nós que estão em constante movimento, porém a velocidade dos nós é alterada a cada 5 segundos.
//A velocidade é alterada partindo de um sinal via wifi do nó 4. Sinal esse que vai repassando de nó em nó até todos terem a mesma velocidade.
//O tempo total da simulação é 30 segundos.



#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TopologiaBarra");

uint16_t porta = 8080;
Ptr<Socket> sendSockets[5];
Ipv4InterfaceContainer interfaces;
Ptr<UniformRandomVariable> randGen;
AnimationInterface *animPtr = nullptr;
void EnviarVelocidade(float velocidade);
void EnviarParaVizinho(uint32_t senderIndex, uint32_t receiverIndex, uint32_t valor);
void ReceberPacote(Ptr<Socket> socket);
bool AcceptCallback(Ptr<Socket>, const Address &) { return true; }
void NovaConexao(Ptr<Socket> socket, const Address &) {
    socket->SetRecvCallback(MakeCallback(&ReceberPacote));
}


void AgendarEnvioVelocidade() {
  if (Simulator::Now().GetSeconds() >= 30.0) return;
  float novaVelocidade = 2 + (randGen->GetInteger(0, 600)/100.0); // 2.00 a 8.00
  NS_LOG_UNCOND("Nó 4 definindo nova velocidade: " << novaVelocidade << " m/s");
  EnviarVelocidade(novaVelocidade);
  
  // Agenda o próximo envio em 5 segundos
  Simulator::Schedule(Seconds(5.0), &AgendarEnvioVelocidade);
}

//Essa é a função principal da implementação
void EnviarVelocidade(float velocidade) {
    uint8_t buffer[5];
    buffer[0] = 1;
    memcpy(&buffer[1], &velocidade, sizeof(float));

    Ptr<Packet> pkt = Create<Packet>(buffer, 5);
  
    Ptr<Node> node4 = sendSockets[4]->GetNode();
    Ptr<ConstantVelocityMobilityModel> mob4 = node4->GetObject<ConstantVelocityMobilityModel>();
    mob4->SetVelocity(Vector(velocidade, 0.0, 0.0));
    NS_LOG_UNCOND("Nó 4 aplicou velocidade a si mesmo: " << velocidade << " m/s");
  
    
    Ptr<Socket> socket = Socket::CreateSocket(node4, TcpSocketFactory::GetTypeId());
    socket->Connect(InetSocketAddress(interfaces.GetAddress(3), porta));
    socket->Send(pkt);
  }
//Não consegui deixar os 30 segundos exatos em tempo de terminal, mas em visualizadores como no netanim é possivel ver que toda simulação realmente dura 30 segundos.
void IniciarEnvio(uint32_t senderIndex, uint32_t receiverIndex) {
    if (Simulator::Now().GetSeconds() >= 30.0) return;

    uint32_t valor = randGen->GetInteger(1, 100);
    NS_LOG_UNCOND("Nó " << senderIndex << " gerou valor " << valor);
    EnviarParaVizinho(senderIndex, receiverIndex, valor);
}



void EnviarParaVizinho(uint32_t senderIndex, uint32_t receiverIndex, uint32_t valor) {
    uint8_t buffer[5];
    buffer[0] = 0;
    memcpy(&buffer[1], &valor, sizeof(uint32_t));
    Ptr<Packet> packet = Create<Packet>(buffer, 5);

    Ptr<Socket> socket = Socket::CreateSocket(sendSockets[senderIndex]->GetNode(), TcpSocketFactory::GetTypeId());
    Ipv4Address destIp = interfaces.GetAddress(receiverIndex);
    InetSocketAddress remote = InetSocketAddress(destIp, porta);
    socket->Connect(remote);
    socket->Send(packet);
}

//Função levemente alterada em relação a função utilizada na topologia de barra estática
void ReceberPacote(Ptr<Socket> socket) {
  Address from;
  Ptr<Packet> packet = socket->RecvFrom(from);
  uint8_t buffer[5];
  packet->CopyData(buffer, 5);

  uint8_t tipo = buffer[0];
  Ptr<Node> thisNode = socket->GetNode();
  uint32_t nodeId = thisNode->GetId();

  if (tipo == 1) {
      float velocidade;
      memcpy(&velocidade, &buffer[1], sizeof(float));
      NS_LOG_UNCOND("Nó " << nodeId << " recebeu velocidade: " << velocidade << " m/s");

      Ptr<ConstantVelocityMobilityModel> mob = thisNode->GetObject<ConstantVelocityMobilityModel>();
      mob->SetVelocity(Vector(velocidade, 0.0, 0.0));

      if (nodeId > 0) { 
          Ptr<Socket> forwardSocket = Socket::CreateSocket(thisNode, TcpSocketFactory::GetTypeId());
          forwardSocket->Connect(InetSocketAddress(interfaces.GetAddress(nodeId - 1), porta));
          Ptr<Packet> forwardPacket = Create<Packet>(buffer, 5);
          forwardSocket->Send(forwardPacket);
      }
  }
}


int main() {
    NodeContainer nodes;
    nodes.Create(5);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(nodes);

    for (uint32_t i = 0; i < 5; ++i) {
        Ptr<ConstantVelocityMobilityModel> mob = nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        mob->SetPosition(Vector(50.0 * i, 0.0, 0.0));
        mob->SetVelocity(Vector(1.0, 0.0, 0.0));
    }

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy = YansWifiPhyHelper();
    phy.SetChannel(channel.Create());

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("DsssRate1Mbps"));

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.0.0.0");
    interfaces = address.Assign(devices);

    // Criação dos sockets, a parte que mais deu trabalho no código junto da implementação da lógica do looping, que aqui está alterado para começar no nó 4.
    for (uint32_t i = 0; i < 5; ++i) {
        sendSockets[i] = Socket::CreateSocket(nodes.Get(i), TcpSocketFactory::GetTypeId());

        Ptr<Socket> recvSocket = Socket::CreateSocket(nodes.Get(i), TcpSocketFactory::GetTypeId());
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), porta);
        recvSocket->Bind(local);
        recvSocket->Listen();
        recvSocket->SetAcceptCallback(MakeCallback(&AcceptCallback), MakeCallback(&NovaConexao));
    }

    randGen = CreateObject<UniformRandomVariable>();
    randGen->SetAttribute("Min", DoubleValue(1));
    randGen->SetAttribute("Max", DoubleValue(100));
    AnimationInterface anim("topologia-movimento.xml"); //Vai gerar um arquivo xml caso queira testar no simulador netanim depois, mas já vou avisando que a visualização no netanim está meio cru.

    for (uint32_t i = 0; i < 5; ++i) {
        anim.SetConstantPosition(nodes.Get(i), 50.0 * i, 20.0);
        anim.UpdateNodeDescription(i, "Nó " + std::to_string(i));
        anim.UpdateNodeColor(i, 0, 255, 0);
    }

    //Não consegui encaixar as imagens para utilizar no netanim, então deixei essa parte caso vc queira mudar e deixar a visualização mais bonitinha.
    uint32_t pcIcon = anim.AddResource("pc.png");
    uint32_t serverIcon = anim.AddResource("server.png");


    anim.UpdateNodeImage(0, serverIcon);
    anim.UpdateNodeImage(1, pcIcon);
    anim.UpdateNodeImage(2, pcIcon);
    anim.UpdateNodeImage(3, pcIcon);
    anim.UpdateNodeImage(4, pcIcon);
    anim.EnablePacketMetadata(true);

    anim.UpdateNodeDescription(0, "Nó 0 - Início");
    anim.UpdateNodeDescription(1, "Nó 1");
    anim.UpdateNodeDescription(2, "Nó 2");
    anim.UpdateNodeDescription(3, "Nó 3");
    anim.UpdateNodeDescription(4, "Nó 4 - Geração");


    Simulator::Schedule(Seconds(5.0), &AgendarEnvioVelocidade);

    Simulator::Stop(Seconds(30.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}