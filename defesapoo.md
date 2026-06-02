# O Guia Supremo da Arquitetura DomusControl

Este é o documento final e estruturado para leres e saberes exatamente onde está cada coisa, porque é que foi feita dessa forma, e o que responder a perguntas capciosas (rasteiras) dos professores. 

---

## 1. O Início de Tudo e a Separação de Camadas (MVC)

### O ficheiro `Main` e a `View`
*   **O que faz?** O `Main` é apenas a porta de entrada. Ele tenta carregar o estado guardado no ficheiro (serialização) ou cria um sistema vazio. Depois, arranca com a `View`.
*   **A Camada View:** A pasta `view` trata de tudo o que é `System.out.print` e `Scanner`. **Regra de Ouro:** A View *nunca* muda um dispositivo diretamente (nunca faz `lampada.setEstado()`). Ela delega sempre esse trabalho para o Model.

### `IDomusControl` (Interface) vs `DomusControlFacade` (Classe)
*   **Porquê uma interface `IDomusControl`?** 
    O professor vai perguntar: *"Para que é que serve esta interface se só tens uma Facade?"*
    **A tua resposta:** *"Serve para reduzir o acoplamento (Coupling). A nossa View só conhece a interface `IDomusControl`. Ela sabe **o que** o sistema pode fazer, mas não sabe **como** a `DomusControlFacade` faz. Isto permite-nos, se quisermos no futuro, reescrever completamente a Facade ou usar um mock para testes, sem alterar uma única linha de código na View."*
*   **DomusControlFacade:** O Padrão *Facade* esconde a complexidade de ter centenas de classes. É o "cérebro" central.

### As Estruturas de Dados Principais (Na Facade)
*   **Porquê usar `Maps` (`Map<UUID, Dispositivo>`, `Map<String, Casa>`)?**
    **A tua resposta:** *"Para máxima eficiência. Os dicionários (Maps) permitem acesso O(1). Em vez de usar um `List` e ter de fazer um `for` a percorrer 100 dispositivos para encontrar aquele cujo ID o utilizador escolheu, o `Map` vai buscar diretamente à memória a referência exata. Usámos UUIDs para garantir identificadores únicos no mundo inteiro."*

---

## 2. A Pasta `dispositivos` (O Hardware Físico)

Se abrires esta pasta, vês subclasses para tudo (Lâmpadas, Colunas, etc). 

*   **Porquê a Classe Abstrata `Dispositivo`?**
    **A tua resposta:** *"Uma Lâmpada e uma Coluna têm coisas em comum: as duas têm um ID, uma marca, um estado (ON/OFF), e ambas podem ser ligadas. Em vez de repetirmos esse código 10 vezes (uma em cada classe), colocámos isso na superclasse `Dispositivo`. Ela é **abstract** porque não faz sentido no mundo real criares 'um dispositivo genérico'; só faz sentido criares uma especialização concreta, como uma `ColunaSom`."*
*   **Subclasses (ex: `ColunaSom`):** Elas fazem `extends Dispositivo`. Herdam o "ligar/desligar", mas têm os seus próprios métodos únicos, como `setVolume()`.
*   **O Padrão Factory (`DispositivoFactory`):** Se a Facade quiser criar uma coluna, ela nunca faz `new ColunaSom()`. Em vez disso, a View passa um Enum (`TipoDispositivo.COLUNA_SOM`) e os parâmetros necessários à Facade. A Facade delega essa informação à Factory. A Factory tem um grande `switch` que analisa o Enum e injeta os parâmetros no construtor correto. Isto centraliza a criação de objetos e segue o princípio **Open/Closed**: se adicionares um novo dispositivo amanhã, a Facade fica intocável; só adicionas um `case` na Factory.
*   **Os Enums (`TipoDispositivo`, `EstadoDispositivo`):** Garantem *Type Safety* (segurança de tipos). Evitam que a View passe strings soltas (como "coluna" ou "LIGADo") que causariam erros de execução, obrigando o compilador a verificar as opções válidas.

---

## 3. A Pasta `operacoes` (A Ponte para a Ação)

*   **Porque é que as pastas repetem o nome dos dispositivos?**
    Porque uma Lâmpada tem ações muito específicas que não interessam a uma Porta (ex: Mudar de Cor). Esta arrumação mantém o código organizado (Alta Coesão).
*   **Porque é que `Operacao` é uma INTERFACE e não uma Classe Abstrata?**
    Isto dita a diferença entre reter estado vs. ditar um comportamento. O `Dispositivo` guarda **estado** (guarda se está ligado ou desligado na memória). Por outro lado, uma `Operacao` é apenas um **contrato de ação** temporário. Ela não guarda estado do sistema; apenas precisa de expor um comportamento: o método `void executar()`. Qualquer classe, seja `AbrirPorta` ou `MudarCor`, assina este contrato. Nas Rotinas, isto permite-nos ter uma `List<Operacao>` (aplicando Polimorfismo) e invocar `executar()` em cada elemento sem precisarmos de saber se é um Ligar ou um Desligar.

*   **A "Viagem do Map" e a `OperacaoFactory` (O Fluxo Técnico)**
    Como é que o sistema sabe que a operação "Alterar Velocidade do AC" precisa de receber um parâmetro (a velocidade) e como é que isso chega à operação concreta?
    1. **Os Enums de Configuração (`TipoOperacao` e `ParametroOperacao`):** O `TipoOperacao.ALTERAR_VELOCIDADE_AC` contém um método `getParametros()` que devolve um conjunto indicando do que precisa (ex: `Set.of(ParametroOperacao.VELOCIDADE_AC)`).
    2. **A Criação na View:** A Interface Gráfica (`DispositivoView`) pergunta a este Enum de que parâmetros ele precisa. Baseado nisso, pede os inputs ao utilizador no terminal. Estes inputs são guardados num "pacote", que é um `Map<ParametroOperacao, Object>` (onde a chave é o Enum de parâmetro e o valor é o que o utilizador escreveu).
    3. **Passagem pela Facade:** A View atira este Mapa (juntamente com o ID do Dispositivo e o `TipoOperacao`) para a `DomusControlFacade`. A Facade, não querendo saber de detalhes de criação, delega a tarefa atirando este Mapa para a `OperacaoFactory`.
    4. **O Padrão Factory a agir:** A `OperacaoFactory` recebe o Mapa, analisa o `TipoOperacao` através de um `switch`, e extrai as variáveis do Mapa com chaves estritas (ex: `params.get(ParametroOperacao.VELOCIDADE_AC)`). De seguida, instancializa a classe concreta (ex: `new AlterarVelocidadeAC()`), passando-lhe no construtor exatamente o valor retirado do Mapa.
    5. **Classe Concreta Independente:** A classe `AlterarVelocidadeAC` em si não faz ideia de como a View funciona nem de onde veio o Mapa. O seu construtor apenas recebe uma `Velocidade` e um `ArCondicionado`, guarda-os como `private`, e espera pacientemente que alguém chame o seu método `executar()`.

---

## 4. A Pasta `rotinas` (A Inteligência do Sistema)

Esta é, sem dúvida, a parte mais rica em arquitetura do teu projeto. 

*   **A Classe Abstrata `Rotina`:**
    Assim como o dispositivo, guarda o comum: o `UUID`, se a rotina está ativa ou não (`estado`), e o mais importante: uma `List<Operacao>`. É **abstrata** porque uma Rotina pura não sabe quando deve ser ativada.
*   **Os Filhos (`Automacao`, `Escalonamento`, `Cenario`):**
    *   `Automacao`: Traz uma condição de sensor (ex: Se SensorMovimento disparar).
    *   `Escalonamento`: Traz uma condição temporal (ex: Se forem 18:00).
    *   `Cenario`: Um conjunto de ações prontas a correr de imediato pelo utilizador.
*   **O Ciclo de Vida: `processar()`, `avaliar()` e `detetar()` (Padrão Template Method)**
    O professor vai querer saber *como* é que o sistema "ganha vida" sozinho. A resposta passa por um padrão de desenho chamado **Template Method** implementado na classe abstrata.
    1. **O Gatilho:** Quando o tempo avança na `Facade`, ela diz a todas as casas: "O tempo passou, vejam se há rotinas a ativar". A Casa itera pela sua lista de rotinas e chama `rotina.processar()`.
    2. **O Esqueleto Genérico (`processar()`):** O método `processar()` está escrito **uma única vez** na superclasse `Rotina`. A sua lógica é incrivelmente simples: `if (this.avaliar()) { this.operacoes.forEach(Operacao::executar); }`. Ou seja, dita o *Esqueleto*: se a condição der verdadeiro, executa a lista de operações.
    3. **O Miolo Específico (`avaliar()`):** O método `avaliar()` é `abstract` na superclasse. Isto força as filhas a definirem a sua própria condição para preencher o "if" lá de cima!
       * Num **Escalonamento**, o `avaliar()` simplesmente pergunta ao Relógio se a hora configurada já bate certo.
       * Numa **Automação**, a condição baseia-se no ambiente exterior. O `avaliar()` vai comunicar com o `Sensor`.
    4. **O Ponto de Contacto com o Mundo Exterior (`detetar()`):** Na Automação, o `avaliar()` invoca `this.sensor.detetar()`. É o `detetar()` que vai fisicamente perguntar ao `SimuladorAmbiente` se, por exemplo, está a chover ou qual é a temperatura atual.
    * **Resumo de Fluxo:** Tempo Avança -> Facade chama Casa -> Casa chama `processar()` da Automação -> O Esqueleto na Superclasse chama o `avaliar()` da Filha -> A Filha chama o `detetar()` do Sensor -> O Sensor lê o Ambiente. Se for `true`, as operações arrancam todas em cadeia.

*   **A Magia dos `Builders` (`EscalonamentoBuilder`, etc):**
    **O que dizer:** *"Implementámos o padrão **Builder** para resolver um problema da Interface de Utilizador. O utilizador não nos dá os dados da Rotina todos no mesmo milissegundo. Ele primeiro escolhe a hora, a seguir navega nos menus, escolhe uma lâmpada, escolhe ligá-la, a seguir escolhe uma porta, e tranca-a. O Builder serve para acumular este estado de forma temporária na Facade. Quando o utilizador clica em 'Concluir', chamamos o método `.build()`, que pega em todos esses bocadinhos e devolve uma `Rotina` íntegra, válida e segura, que inserimos na Casa."*

---

## 5. Relógio e Simulador de Ambiente

*   Ambos usam o Padrão **Singleton**. 
*   **Porquê?** *"A simulação depende da passagem coerente do tempo. Se pudéssemos instanciar vários relógios com horas diferentes, o sistema enlouquecia. O Singleton garante que todo o código acede à mesma exata linha temporal (`getInstance()`). Quando avançamos o relógio global, as rotinas de todas as casas são ativadas em concordância."*

---

## 6. Eventos e o Padrão Observer + Decorator (O Histórico em Tempo Real)

O sistema de eventos (`RegistoEventos`) não é apenas uma lista de strings. Ele utiliza dois padrões de desenho avançados que revelam grande maturidade arquitetural e que deves compreender a fundo:

*   **O Padrão Decorator (`OperacaoNotificadora`):** 
    Como é que o sistema sabe que deve registar um evento sempre que uma Lâmpada é ligada? Em vez de "sujarmos" o código de dezenas de classes diferentes (como a `LigarDispositivo`) com lógica de histórico, usamos o padrão **Decorator**. A Facade pega na operação original puramente funcional (`LigarDispositivo`) e "embrulha-a" dinamicamente dentro de uma classe `OperacaoNotificadora`. 
    Quando o método `executar()` do decorador é chamado no momento da ação, ele intercepta a execução e faz três coisas:
    1. Regista o estado do dispositivo *antes* da ação (chamando `toString()`).
    2. Chama o `executar()` da operação original embrulhada (a lâmpada liga fisicamente na memória).
    3. Verifica o estado *depois* da ação. Se as strings forem diferentes (ou seja, se a lâmpada realmente mudou e não estava já ligada), ele constrói um objeto `Evento` e guarda-o no Registo.

*   **O Padrão Observer (`Observer` e `RegistoEventos`):**
    O `RegistoEventos` da casa atua como um **Publisher** (Publicador). Ele não só guarda um `List<Evento> historico` estático, como também mantém uma `List<Observer> observers`.
    1. **O Contrato (`Observer.java`):** É uma interface minúscula com apenas o método `void update(Evento evento)`. Qualquer parte do código (seja uma UI gráfica avançada, um ficheiro de logs num servidor, ou outra funcionalidade) pode assinar este contrato.
    2. **Subscrição:** Essa entidade externa diz ao `RegistoEventos`: *"Avisa-me sempre que algo acontecer"* através do método `adicionarObserver(Observer o)`.
    3. **Notificação Automática:** Sempre que a `OperacaoNotificadora` atira um novo evento para o registo via `adicionarEvento(e)`, a lista faz logo um ciclo iterando sobre todos os seus observers registados e chamando o `o.update(e)` em todos eles. Isto é a definição perfeita de **Baixo Acoplamento**: o Registo de Eventos da Casa avisa quem quiser ser avisado em "tempo real", sem fazer a mais pequena ideia de *quem* são os subscritores e do que eles fazem com a informação.

## 7. Utilizadores e Exceções (Segurança e Isolamento)

*   **Hierarquia de Utilizadores:** Existe a classe base `Utilizador` que garante o encapsulamento das credenciais (email e hash de password) e tem o seu comportamento lógico especializado pela classe filha `Proprietario` (recorrendo ao princípio da Herança). O `Proprietario` expande as funcionalidades base acrescentando a lógica de possuir propriedades (Casas).
*   **Gestão por Exceptions (Exceções):** O Model foi programado num paradigma defensivo e **nunca** comunica diretamente com o terminal (não usa `System.out.println` nem lê `Scanner`). Quando um erro lógico ocorre num processo (ex: um dispositivo que não existe, erro `DispositivoInvalidoException`), a classe do Model pura e simplesmente levanta a Exceção para cima. As classes da View possuem blocos `try-catch` construídos para apanhar essas Exceções do modelo e apresentá-las ao utilizador final. Isto consagra o **Princípio da Responsabilidade Única (SRP)**: O Model trata de regras de negócio; a View trata apenas do ecrã e recolha de teclas.

---

## 8. Como se Testam estas ligações? (O Segredo do `Dummy`)

Se abrires a pasta dos `tests` (ex: `CenarioTest.java`), vês que criaram uma `DummyOperacao`.
*   **O que é isso e porquê?**
    **A tua resposta:** *"Quando escrevemos Testes Unitários para uma Rotina (Cenário), nós só queremos testar se o Cenário consegue executar operações em cadeia. Nós não queremos instanciar lâmpadas, portas e construtores complexos. Então, criamos um **Dummy/Mock** local: uma classe falsa que implementa a interface `Operacao` e que o método `executar()` apenas mete uma variável `boolean executada = true`. Quando testamos o Cenário, passamos-lhe o Dummy. Se no final do teste `executada` for `true`, provámos que a classe Cenário cumpre a sua responsabilidade, com acoplamento nulo."*

---

## 9. Composição vs Agregação (Como os Objetos se Relacionam)

Esta é uma pergunta clássica para destrinçar quem realmente sabe modelar software. Ao contrário do que pensavas, o projeto usa **Agregação** para as estruturas principais e **Composição** apenas onde faz estritamente sentido lógico.

1. **A Agregação (Relação Fraca - "Tem Um"):**
   * **Exemplo:** A relação entre `Divisao` e `Dispositivo`.
   * **O que é:** O dispositivo é criado pela Factory e guardado no mapa global da Facade (`Facade.dispositivos`). Só depois é que ele é meramente *associado* a uma `Divisao`. 
   * **Porquê Agregação:** Porque o ciclo de vida do dispositivo **não depende** da divisão. Se, hipoteticamente, a divisão for apagada, o objeto `Dispositivo` continua a existir perfeitamente na memória (no mapa da Facade). O dispositivo consegue "viver" de forma independente da divisão onde foi colocado. Isto é Agregação.

2. **A Composição (Relação Forte - "É Dono De"):**
   * **Exemplo:** A relação entre `Rotina` e `Operacao`, ou `Casa` e `RegistoEventos`.
   * **O que é:** A Rotina tem uma `List<Operacao>`. Estas operações foram instanciadas especificamente para viver dentro desta rotina.
   * **Porquê Composição:** Porque se tu apagares a Rotina, a lista de operações que estava lá dentro é destruída e limpa da memória (recolhida pelo *Garbage Collector*). As operações não foram guardadas num mapa global na Facade. O tempo de vida da `Operacao` depende a 100% da `Rotina`. A Rotina "é dona" da Operação.

---

## 10. As Estatísticas e a Magia das "Streams"

O projeto tem uma parte de cálculo de estatísticas (como a "Casa Mais Consumidora" ou "Dispositivos Mais Ativados"). O professor vai inevitavelmente perguntar como fizeste essas contas no meio de milhares de objetos guardados em `Maps`.

**A tua resposta:** *"Em vez de usarmos os clássicos ciclos `for` imperativos com variáveis acumuladoras (que tornam o código extenso e mais sujeito a bugs), optámos pela abordagem declarativa e funcional através da **Java Streams API**."*

* **Como funciona (Exemplo da `casaMaisConsumidora`):** 
  ```java
  public Casa casaMaisConsumidora() {
      return this.casas.values().stream()
              .max(Comparator.comparingDouble(Casa::getConsumoTotal))
              .map(Casa::clone)
              .orElse(null);
  }
  ```
  * O código transforma a coleção dos dicionários num fluxo de dados encadeado (`.stream()`).
  * Usa um comparador nativo (`Comparator.comparingDouble`) conjugado com _Method References_ (`Casa::getConsumoTotal`) para extrair o valor de cada objeto e encontrar o maior.
  * Retorna o próprio objeto (`.map(Casa::clone)`) de forma segura, usando as cópias defensivas que falámos acima.
  * O `.orElse(null)` trata imediatamente o caso de o mapa estar vazio, evitando o clássico e temido erro `NullPointerException`. É código limpo que diz **o que** queremos (o máximo), e não **como** iterar passo a passo para o descobrir.

---

### Resumo para estudares 5 minutos antes de entrares:
1. **O que é o Model?** Onde está a lógica. NUNCA tem prints.
2. **O que é a Facade?** A nossa porteira. A View só fala com ela.
3. **Dispositivo vs Operacao:** Dispositivo é Abstract porque tem estado partilhado (ID, estado ON/OFF). Operacao é Interface porque é só um contrato puramente abstrato (`executar()`).
4. **Porque usaste Builders?** Porque a View insere as rotinas passo-a-passo.
5. **Onde guardas os objetos?** Em `Maps` na Facade, usando `UUIDs`, porque o acesso O(1) bate a procura linear das `Lists`.
