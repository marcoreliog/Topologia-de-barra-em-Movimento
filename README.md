# Simulação de Topologia com Movimento Dinâmico no NS-3

Este projeto implementa uma simulação no simulador de redes **NS-3**, com foco na mobilidade de nós em uma topologia linear (em barra), onde os nós se movimentam e atualizam suas velocidades de forma coordenada via comunicação Wi-Fi.

## 🧠 Descrição do Projeto

- A simulação contém **5 nós** dispostos em linha reta.
- Todos os nós estão em **constante movimento**, inicialmente com velocidade de `1.0 m/s`.
- A **cada 5 segundos**, o **nó 4** gera uma nova velocidade aleatória entre `2.00 m/s` e `8.00 m/s`.
- Essa velocidade é transmitida em cadeia para os nós vizinhos, até todos os nós estarem sincronizados com a nova velocidade.
- A transmissão ocorre por meio de **sockets TCP** via **Wi-Fi Ad-hoc**.
- A **duração total** da simulação é de **30 segundos**.
- Foi utilizado o **NetAnim** para visualização do movimento e comunicação entre os nós.

## 🔧 Estrutura do Código

### Principais Componentes:

- `EnviarVelocidade(float velocidade)`: função chamada pelo nó 4 para definir e enviar uma nova velocidade.
- `ReceberPacote(...)`: recebe pacotes e aplica a velocidade recebida ao nó, além de repassá-la ao próximo.
- `AgendarEnvioVelocidade()`: agenda o envio periódico de novas velocidades.
- `main()`: configuração de nós, mobilidade, Wi-Fi, sockets e animação.

## 🛠️ Tecnologias Utilizadas

- [NS-3](https://www.nsnam.org/)
- Módulos: `core`, `network`, `internet`, `mobility`, `wifi`, `applications`, `netanim`
- Modelo de mobilidade: `ConstantVelocityMobilityModel`
- Comunicação: `TCP sobre Wi-Fi Ad-hoc`
- Visualização: `NetAnim`

## 📦 Arquivo de Saída

- A simulação gera o arquivo `topologia-movimento.xml`, que pode ser aberto no NetAnim para visualização da movimentação dos nós.

## 🎨 Visual no NetAnim

- Cada nó possui uma descrição personalizada (ex: "Nó 0 - Início", "Nó 4 - Geração").
- Ícones foram configurados (embora não tenha sido possível incluir imagens no momento).
- Cores e posições são ajustadas para melhor entendimento da topologia.

## 🚀 Execução

Para executar esta simulação, siga os seguintes passos:

1. Certifique-se de que o NS-3 está corretamente instalado.
2. Compile o código com `waf`:
   ```bash
   ./waf configure
   ./waf build
   ./waf --run nome-do-seu-script
   ```
3. Abra o arquivo `topologia-movimento.xml` no **NetAnim**:
   ```bash
   netanim
   ```

## 📌 Observações

- A sincronização de tempo entre terminal e animação visual pode variar, mas no NetAnim é possível observar a simulação completa de 30 segundos.
- O código pode ser estendido para permitir controle mais avançado de mobilidade, múltiplas fontes de velocidade ou cenários com perda de pacotes.
