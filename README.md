# Trabalho3

Victor Putrich, Lucca Dornelles e Guilherme Santos

# Sobre

## Introdução
Este relatório visa apresentar e avaliar os resultados das execuções de um programa multithread e seus diferentes escalonamentos de acordo com a mudança de politica de escalonamento selecionada, além da mudança em ambientes de processamento multicore e single-core.

## Objetivo
O objetivo deste trabalho é a criação de um programa multithread com um número específico de threads que escrevem caracteres em um buffer global. A escrita no buffer é sincronizada, de forma que não ocorra sobrescrita de uma posição, pois assim é possivel verificar o escalonamento das threads durante a execução através da leitura do conteudo do buffer. Para facilitar a analise, ao concluir a execução o programa mostra a leitura do buffer e a contagem de quantas vezes cada caracter foi escalonado.

# Funcionamento

O programa simula o processamento consecutivo e intercalado de vários processos utilizando multiplas threads que escrevem em um buffer. A ordem e tempo reservado a cada thread é dependente do scheduler utilizado. 

### Mutex
Utilizamos dois objetos mutex para coordenar o funcionamento das threads. 
**simLock** trava qualquer thread que tente rodar antes que todas sejam inicializadas, fazendo com que todas threads começem simultaneamente. 
**lock** é utilizado dentro das threads para impedir que uma thread rode enquanto outra esteja na zona crítica (que é a interidade da thread após sua inicialização). Enquanto uma thread está escrevendo no buffer todas as outras esperam e só após o processo de escrita acabar que o scheduler pode trocar de thread. 

### Output
O output retornado pelo processo é uma lista de letras com letras duplicadas removidas (e.g. aaba -> aba), que representa a ordem de execução das threads. 

## Testes
Para a execução dos testes foram criados dois escripts o "runm" e "runs". O primeiro executa o programa "testm.c" que possibilita a execução do programa multithread configurando a politica de escalonamento para cada fila. Já o segundo executa o "tests.c" que possibilita o teste configurando uma politica de escalonamento para todas as filas.
