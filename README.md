# Projeto Bloxorz (Com Textura & Iluminação) - Equipe 5

### Projeto em Web Assembly

Para acessar, [clique aqui](./public/cube_trail.html).

### Integrantes

- Gabriel Garcia de Lima - R.A.: 11201932522
- Fernando Hiroaki Suzuki - R.A.: 11202130281
- Leonardo Fabiano de Sousa - R.A.: 11201721317

### Introdução

Nosso projeto é uma implementação do clássico jogo de lógica e estratégia Bloxorz, onde o objetivo é guiar um bloco retangular através de um tabuleiro até encaixá-lo verticalmente em um buraco. A mecânica envolve movimentação estratégica, evitando cair para fora do tabuleiro e explorando diferentes orientações do bloco durante os movimentos. Esta implementação utiliza a biblioteca OpenGL para renderização gráfica e interação 3D.

## Guia de Utilização da Aplicação Final

1. O jogador controla um bloco 3D retangular que pode estar em três estados principais:

   - **Em pé (STANDING)**: O bloco ocupa um único quadrado do tabuleiro.
   - **Deitado no eixo X (LAYING_X)**: O bloco ocupa dois quadrados adjacentes horizontalmente.
   - **Deitado no eixo Z (LAYING_Z)**: O bloco ocupa dois quadrados adjacentes verticalmente.

2. O tabuleiro é uma grade quadriculada que contém um buraco. O bloco deve ser posicionado exatamente em pé sobre o buraco para vencer.

3. Movimentos são realizados em quatro direções (cima, baixo, esquerda, direita). Cada movimento altera a orientação ou posição do bloco.

4. Se o bloco cair para fora do tabuleiro ou não estiver corretamente alinhado, o jogo reinicia.

5. O usuário 'vence' se conseguir fazer com que o bloco caia alinhado verticalmente dentro do burado gerado de forma randomizada.

---

## Projeto e Desenvolvimento

### Funcionalidades

- **Movimentação do Bloco**: O bloco pode se mover de forma fluida, com rotações e animações calculadas dinamicamente.
- **Tabuleiro Dinâmico**: Um buraco é gerado aleatoriamente no início de cada partida.
- **Detecção de Vitória e Derrota**:
  - Vitória: Quando o bloco está em pé exatamente sobre o buraco.
  - Derrota: Quando o bloco cai fora do tabuleiro ou não se ajusta ao buraco.
- **Animações Suaves**: O jogo utiliza transformações matriciais para animar os movimentos do bloco, garantindo uma experiência visual fluida.
- **Sistema de Reposicionamento**: Ao reiniciar, o bloco é posicionado em uma nova localização válida, enquanto o buraco é reposicionado de forma aleatória.

---

### Controles

- **Teclas de Direção**:
  - **W** ou **Seta para Cima**: Move o bloco para cima.
  - **S** ou **Seta para Baixo**: Move o bloco para baixo.
  - **A** ou **Seta para Esquerda**: Move o bloco para a esquerda.
  - **D** ou **Seta para Direita**: Move o bloco para a direita.

---

### Estrutura do Código

O projeto está dividido em três componentes principais, cada um encapsulado em um arquivo `.cpp`.

### Arquivos Principais

1. **`cube.cpp`**: Gerencia as funcionalidades relacionadas ao bloco, incluindo:
   - Movimentação.
   - Controle de estados e animações.
   - Detecção de queda e resete.
2. **`ground.cpp`**: Lida com a lógica do tabuleiro, como:
   - Verificação de limites.
   - Gerenciamento do buraco (posição e validade).
   - Renderização do tabuleiro.
3. **`window.cpp`**: Controla a interação entre o usuário e o jogo:
   - Processamento de eventos do teclado.
   - Integração entre o bloco e o tabuleiro.
   - Configuração da renderização e animação.

---

### Detalhes da Lógica

1.  **Movimentação e Estados do Bloco**:

    - O estado do bloco determina como ele se move e ocupa o tabuleiro.
    - As rotações são calculadas com base em eixos de rotação e pontos de pivô.
    - Ao atingir o limite do tabuleiro ou o buraco incorretamente, o estado de queda é acionado.

2.  **Gerenciamento do Tabuleiro**:

    - O tabuleiro é gerado como uma matriz 2D. Cada posição pode ser uma célula válida ou o buraco.
    - A função `isTile()` garante que o bloco não ultrapasse os limites ou caia em locais inválidos.

3.  **Detecção de Vitória/Derrota**:

    - **Vitória**: O bloco está em pé (`STANDING`) e sua posição coincide com a do buraco.
    - **Derrota**: O bloco ultrapassa os limites ou não se posiciona corretamente sobre o buraco.

4.  **Animação Suave**:

    - Rotações e translações do bloco são calculadas frame a frame usando transformações matriciais para garantir fluidez.

5.  **Aleatoriedade no Jogo**:

    - O buraco é gerado em posições aleatórias, garantindo que cada partida seja única.

6.  **Aplicação de Texturas**:

    > Para que o código do Bloxorz fosse compatível com a entrada de texturas, coordenadas UV foram adicionadas ao arquivo de definição `box.obj`. Essas coordenadas mapeiam os vértices do tabuleiro e do bloco, permitindo uma aplicação correta das textura.

    A função `loadObj` processa os dados do modelo carregado (neste caso, `box.obj`) e extrai coordenadas de textura UV. Essas coordenadas são armazenadas no atributo texCoord de cada vértice do modelo:

    ```c++
    glm::vec2 texCoord{};
    if (index.texcoord_index >= 0) {
     auto const texStartIndex{2 * index.texcoord_index};
     texCoord = {attrib.texcoords.at(texStartIndex + 0),
                 attrib.texcoords.at(texStartIndex + 1)};
    }
    ```

    **Obs:** Isso permite que cada vértice seja associado a uma parte específica da textura.

    Durante a renderização do cubo na função `paint`, a textura é vinculada antes de desenhar os elementos:

    ```c++
    abcg::glBindTexture(GL_TEXTURE_2D, m_texture);
    abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
    }
    ```

    **Obs:** Isso garante que a textura seja aplicada corretamente ao modelo renderizado.

    Na função `create`, o atributo `inTexCoord` é vinculado ao buffer de vértices para habilitar o uso de texturas nos shaders:

    ```c++
    auto const texCoordAttribute{abcg::glGetAttribLocation(program, "inTexCoord")};
    if (texCoordAttribute >= 0) {
        abcg::glEnableVertexAttribArray(texCoordAttribute);
        abcg::glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE,
                                    sizeof(Vertex),
                                    reinterpret_cast<void*>(offsetof(Vertex, texCoord)));
    }
    ```

    Texturas carregadas no `window.cpp`. A função `loadTexture` carrega arquivos de textura com parâmetros configuráveis. A textura do cubo é configurada na função `onCreate` usando `cubeTexture`:

    ```c++
    //[Função onCreate]
    //...
    // Carrega as texturas para o chão e para o cubo
    auto groundTexture = loadTexture(assetsPath + "tileTexture03.jpg");
    auto cubeTexture   = loadTexture(assetsPath + "cubeTexture03.jpg");

    // Cria o chão e o cubo
    m_ground.create(m_program, m_modelMatrixLoc, m_colorLoc, m_scale, m_N);
    m_ground.setTexture(groundTexture);

    m_cube.loadObj(assetsPath + "box.obj");
    m_cube.create(m_program, m_modelMatrixLoc, m_colorLoc, m_viewMatrix, m_scale, m_N);
    m_cube.setTexture(cubeTexture);
    //...
    ```

7.  **Iluminação**:

    Configuração no window.cpp, função `onPaint`. Variáveis uniformes definem direção, cor e intensidade da luz. `lightDir` determina a direção da luz principal. Definida como:

    ```c++
    glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
    abcg::glUniform3fv(lightDirLoc, 1, &lightDir.x);

    //lightColor ajusta a intensidade da luz
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    abcg::glUniform3fv(lightColorLoc, 1, &lightColor.x);

    //ambientColor configura a iluminação ambiente
    glm::vec3 ambientColor = glm::vec3(0.6f, 0.6f, 0.6f);
    abcg::glUniform3fv(ambientColorLoc, 1, &ambientColor.x);
    ```

    **Obs:** Esses parâmetros interagem diretamente com os shaders `texture_light.vert` e `texture_light.frag`.

---

## Problemas e Soluções

1. Sincronização de texturas:

   - **Problema**: As texturas não eram aplicadas corretamente ao modelo;
   - **Solução**: Revisão dos atributos de vértice em cube.cpp e correção do carregamento de UV.

2. Configuração da iluminação:

   - **Problema**: A iluminação parecia muito escura ou superexposta;
   - **Solução**: Ajustes nos parâmetros de ambientColor e lightDir no window.cpp.

## Resultados e Análise

1. **Avaliação Computacional e de Desempenho**: O jogo demonstrou bom desempenho, mantendo uma taxa alta de FPS rodando como exe local, bem como compilado em _WebAssembly_;

2. **Resultado Visual e Interatividade**: As texturas criaram um ambiente visualmente agradável e coerente. A interação com o cubo é fluida o suficiente, proporcionando uma experiência bacana.

### Avaliação de Usuários

1. **Feedback Visual**: Amigos e parentes elogiaram o jogo, bem como a proposta. O Feedback foi de que o Bloxorz que desenvolvemos é divertido, estimula a mente e ajuda a passar o tempo;
2. **Interatividade**: Movimentação intuitiva e responsiva foi bem recebida.

## Conclusões

### Destaques e Contribuições

O objetivo final foi atingido: criar um jogo casual que estimula a mente a pensar em caminhos eficientes para alinhar o cubo ao buraco. Além disso, a texturização e iluminação aplicadas proporcionaram mais realismo e profundidade ao projeto.

### Limitações

Trabalhar com as coordenadas do cubo à medida que ele se afastava do eixo principal do tabuleiro apresentou uma alta complexidade para o time. Algumas distorções ocasionais ainda podem ocorrer durante a movimentação.

### Trabalho Futuro

1. Corrigir as distorções observadas na movimentação do cubo relacionadas às coordenadas;
2. Proporcionar tabuleiros com formatos randomizados, não apenas quadrados, aumentando o nível de desafio e engajamento do usuário;
3. Expansão do cenário com mais elementos interativos.

## Referências

[MCTA008-17 Computação Gráfica - Prof. Harlen Batagelo](https://hbatagelo.github.io/cg/index.html);

---

Para mais detalhes sobre a lógica ou a estrutura de implementação, consulte os arquivos de código anexados.
