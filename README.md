# Trabalho3

Victor Putrich, Lucca Dornelles e Guilherme Santos

<!--ts-->
   * [Introdução](#Introdução)
   * [Escalonadores](#Escalonadores)
   * [Implementação](#Implementação)
   * [Testes](#Testes)
<!--te-->


# Introdução
Este relatório visa apresentar e avaliar os resultados das execuções de um programa multithread e seus diferentes escalonamentos de acordo com a mudança de politica de escalonamento selecionada, além da mudança em ambientes de processamento multicore e single-core.

O objetivo deste trabalho é a criação de um programa multithread com um número específico de threads que escrevem caracteres em um buffer global. A escrita no buffer é sincronizada, de forma que não ocorra sobrescrita de uma posição, pois assim é possivel verificar o escalonamento das threads durante a execução através da leitura do conteudo do buffer. Para facilitar a analise, ao concluir a execução o programa mostra a leitura do buffer e a contagem de quantas vezes cada caracter foi escalonado.

# Escalonadores
No nosso programa existe um grupo de escalonadores que podem ser utilizados, estes estão divididos em *Completely Fair Scheduler* (CFS) e *Real-time scheduler*. Dentro dessas duas classes, existem diferentes políticas disponibilizadas pelo sistema operacional que podem ser explicitamente definidas ao abrir um thread na execução do programa. Algumas políticas ainda tem disponível um intervalo de prioridade que varia de política para política. O escalonador usa a política aliada à prioridade *sched_priority* para tomar a decisão de qual thread deve ganhar o uso da cpu.

## CFS
As três políticas CSF disponibilizadas para uso no sistemas Linux não tem diferenciação através do parâmetro *sched_priority*, portanto todos são passados com valor 0.

  * **SCHED_OTHER:** Esta é a política default do Linux. Não existe uma prioridade que pode ser definida de forma estática para os processos, ao invés disso as threads que forem usar essa política, tem um valor de prioridade dinâmica chamado "nice value". O objetivo desse atributo é garantir que todas as threads possam vir a "progredir" de forma justa. O valor é ajustado para cada "quantum" que as threads estão em estado *ready*.
  
  * **SCHED_BATCH:** Semelhante à política anterior, o escalonador de *batch* também usa o atributo dinâmico *nice value* para buscar balancear a execução das threads. A diferença dessa política é que ela penaliza tarefas que involvam serem "despertadas". Portanto, é interessante usá-la em tarefas que não sejam tão interativas com o usuário. Tarefas que buscam um escalonamento mais determinístico que não seja dependente de interação, podem se beneficiar desta política. 
  
  * **SCHED_IDLE:** Nesta política não há prioridade estática ou dinâmica, tarefas que usam essa política tem como objetivo ser de baixíssima prioridade.

## Real-time

Aquelas threads que usam as políticas real-time fazem uso do *sched_priority* que tem o intervalo de 1 a 99. Sendo 1 o valor mínimo e 99 o valor máximo. A diferença entre as duas políticas não está entre threads com diferentes prioridades, a thread com maior prioridade sempre vai ganhar acesso à cpu, preemptando a thread corrente. A mudança entre elas está na decisão do que fazer entre threads que tem a mesma prioridade e em que posição na fila adicionar novas threads com diferentes prioridades. Importante comentar que threads que tiverem usando políticas real-time sempre terão preferência sobre a políticas CFS.

  * **SCHED_FIFO:** A política *First in first out*(FIFO) basicamente adiciona os processos a serem executados por ordem de chegada e são alocados de mesma forma. Uma thread que for preemptada por outra thread de prioridade maior, permanecerá na frente da fila e retornará à execução assim que a thread de maior prioridade for bloqueada. A thread que for bloqueada ao voltar para o estado *runnable*, volta para o final da fila. Além disso, threads de maior preferência vão "furar" a fila até a posição onde exista uma thread de maior prioridade a sua frente.

  * **SCHED_RR:** *Round-robin* é uma política mais "justa" se compararmos com a FIFO, sua política é menos sucetível a ocorrência de *starvation*. Diferente da política anterior que roda seus processos até que estes ou sejam preemptados, ou fiquem bloquados, a round-robin atribui um "tempo de execução" para cada thread, chamado de *quantum*. Quando um processo atinge o limite do seu *quantum* é posto no final da fila, de acordo com sua prioridade.
  
  * **SCHED_DEADLINE** A política em questão é usado para o cumprimento das chamadas *sporadic tasks* que é composta por um conjunto de *jobs*. São executados um *job* por vez, e cada um tempo um tempon de *deadline* que seria o momento que o programa a ser executado, caso já não tenha realizado o seu trabalho, acaba descartando todo trabalho feito até em tão. Cada *job* a ser executado é composto por duas características, a *relative deadline* e o tempo de execução. A chamada *absolute deadline* é a soma destes dois valores.
  
# Implementação

## Argumentos_de_Entrada
Para executarmos a nossa aplicação precisamos informar primeiro o número de threads usadas e o tamanho da memório, além disso, para cada thread precisamos explicitamente definir qual a política, podendo ser qualquer uma das listadas anteriormente e a sua respectiva prioridade que varia de política para política. Podemos verificar o formato da entrada logo abaixo:

```
  ./setpriority <NUMBER_THREADS> <MEM_SIZE> <POLICY 0> <PRIORITY 0> ... <POLICY N> <PRIORITY N>
```
 
Podemos rodar por exemplo 4 threads com a política round-robin, todas com prioridade mínima e com memória de tamanho 100.000 da seguinte forma:
  
```
  ./setpriority 4 100000 SCHED_RR 1 SCHED_RR 1 SCHED_RR 1 SCHED_RR 1 SCHED_RR 1 SCHED_RR 1
```

## Função_Run

Para que pudéssemos acompanhar as políticas de escalonamento competindo pelo controle da CPU para escrever a sua letra na memória, definimos dentro da função "run" o método de acesso da memória que pode ser vista por todas as threads do programa. Pimeiramente, para que todas as threads concorram pelo acesso à cpu de forma justa, no início da função usamos um método de sincronização via *mutex* que será explicado posteriormente, isto faz com que todas threads iniciem o processamento de fato juntas. Cada thread recebe uma letra que é passada pelo parâmetro *void data* da função. Este parâmetro é um valor de deslocamento que é incrementado ao caracter inicial do alfabeto. Como o programa foi escrito em C, basta somarmos a 'a' o valor de offset e teremos o caractere da thread correspondente. 

```
  char letter = 'a'+(int)data;
```

A parte principal do programa consiste num while que roda enquanto a memória não estiver preenchida por completo, isso é feito comparado o tamanho máximo de memória *SIZE_BUF* com um contador que serve para sempre apontar para a próxima posição de memória a ser escrita *index_mem*. O while ocorre enquanto o index for menor ou igual ao tamanho máximo de memória. 


### Controle de Acesso
Utilizamos dois objetos mutex para coordenar o funcionamento das threads. 

 * **simLock** trava qualquer thread que tente rodar antes que todas sejam inicializadas, fazendo com que todas threads comecem simultaneamente. 
 * **lock** é utilizado dentro das threads para impedir que uma thread rode enquanto outra esteja na zona crítica (que é a interidade da thread após sua inicialização). Enquanto uma thread está escrevendo no buffer todas as outras esperam e só após o processo de escrita acabar que o scheduler pode trocar de thread. 

Perceba que *index_mem* e a memória em sí são consultadas e escritas por múltiplas threads, para evitar que exista Condição de Corrida no nosso programa, adicionamos uma restrição de acesso durante a escrita em memória e incremento da variável *index_mem*. Dessa forma evitamos por exemplo que duas threads acabem escrevendo na mesma posição de memória ou incrementando o index de forma duplicada, fazendo com que ele incremente somenete uma vez ao invés de duas.

Podemos ver a seguir a implementação da função por completo:

```
void *run(void *data)
{
	//iniciar todas threads "simultaneamente"
	pthread_mutex_lock(&simLock);
	pthread_mutex_unlock(&simLock);
	
	char d = 'a'+(int)data;
	while (index_mem <= SIZE_BUF){		
      // zona crítica
		pthread_mutex_lock(&lock);
		mem[index_mem] = d;
		index_mem+=1;
		pthread_mutex_unlock(&lock);
	}
	pthread_mutex_unlock(&lock);	
	return 0;
}
```



## Scripts_Usados


# Testes

Para os testes rodados, primeiro foi definido um critério de análise das políticas. Como tempos uma gama de escalonadores e podemos usar diferentes métodos em diferentes threads, vamos definir primeiro situações interessantes para avaliarmos:

 * **Single-core:** Para que possamos verificar o comportamento puro de cada política, devemos executá-las em single-core para um número pequeno de threads. Dessa maneira, se usarmos, por exemplo uma mesma política real-time com threads diferentes, porém variando o valor de prioridade, poderemos verificar a organização da fila das threads, assim como a execução da mesma e troca de contexto. Nesse caso, executará somente a thread com maior prioridade, independete se for FIFO ou RR.

 * **Multi-core:** Usando mais de uma cpu na execução do programa, podemos avaliar como o escalonado atribui para cada cpu os processos a partir da sua política e prioridade. Dessa vez, tomando o exemplo anterior, a thread de maior prioridade deve ficar ocupando uma das cpu o tempo todo, enquanto as outras threads que tenham prioridade inferior e igual, devem disputar o uso da cpu disponível.

 * **Política Igual:** Vamos testar também testes com multiplas threads execurando a mesma política, poderemos verificar se o escalonamento condiz com a descrição do comportamento de cada política conforme descrito anteriormente.
 
 * **Classes Diferentes:** Veremos como o escalonador se comporta quando rodamos threads que estão usam alguma das políticas da classe CFS, assim como políticas de classe real-time. Podemoremos acompanhar o escalonamento destas usando 1, 2 ou + cpus. 

## Teste 1

A tabela abaixo descreve a configuração usada para este teste, nele testaremos o comportamento do escalonador mesclando 4 escalonados CSF Other, com uma política real-time com prioridade máxima com 2 CPUs. O comportamento esperado é que uma das cpus fique 100% do tempo ocupada pela política RR e a outra utilize a sua lógica de escalonamento para escalonar as diferentes threads:

CPU|MEM(kb)|Threads|th1|th2|th3|th4|
--- | --- | --- |  --- | --- |  --- | --- |
1|1000000|4|OTHER 0|OTHER 0|OTHER 0|OTHER 0|

Contagem letras:

- (PID 12252) a: 249288KB
- (PID 12253) b: 250688KB
- (PID 12254) c: 251130KB
- (PID 12255) d: 248892KB

 <p float="left">
  <img src="https://github.com/schererl/Trabalho3/blob/master/logs/Teste1/ks-img.png" width="800">
  
  <img src="https://github.com/schererl/Trabalho3/blob/master/logs/Teste1/colors.png" width="50"/>
 </p>
 

## Teste 2 

Neste teste foram utilizadas 4 threads com prioridades iguais e escalonadores batch. Como esperado as três primeiras threads obtiveram tempos semelhantes, porém a quarta thread obteve um tempo muito menor, o que possívelmente seria balanceado caso a execução do programa continuasse. 

CPU|MEM(kb)|Threads|th1|th2|th3|th4|
--- | --- | --- |  --- | --- |  --- | --- |
1|10000|4|BATCH 0|BATCH 0|BATCH 0|BATCH 0|

Contagem letras:

- (PID 10156) a: 3442KB
- (PID 10157) b: 3167KB
- (PID 10158) c: 3150KB
- (PID 10159) d: 239KB

 <p float="left">
  <img src="https://github.com/schererl/Trabalho3/blob/master/logs/Teste2/KS-img.png" width="800">
  
  <img src="https://github.com/schererl/Trabalho3/blob/master/logs/Teste2/colors.png" width="50"/>
 </p>
 
 
## Teste 3

O terceiro teste utiliza a politica FIFO que roda o primeiro processo até ele acabar, for interrompido, ou preemptado por outro com prioridade maior. Como as prioridades são iguais só o primeiro processo conseguiu rodar, o que é o esperado.

CPU|MEM(kb)|Threads|th1|th2|th3|th4|
--- | --- | --- |  --- | --- |  --- | --- |
1|100000|4|FIFO 1|FIFO 1|FIFO 1|FIFO 1|

Contagem letras:

- (PID 10662) a: 100000KB
- (PID 10663) b: 0KB
- (PID 10664) c: 0KB
- (PID 10665) d: 0KB
 
 <p float="left">
  <img src="https://github.com/schererl/Trabalho3/blob/master/logs/Teste3/ks-img.png" width="650">
  
  <img src="https://github.com/schererl/Trabalho3/blob/master/logs/Teste3/colors.png" width="50"/>
 </p>

## Teste 4

No teste do Round Robin foram utilizadas 4 threads com prioridade igual. A execução dos processos foi parelha, assim como no batch, mas com tempos de execução menores devido ao quantum

CPU|MEM(kb)|Threads|th1|th2|th3|th4|
--- | --- | --- |  --- | --- |  --- | --- |
1|1000000|4|SCHED_RR 99|SCHED_RR 99|SCHED_RR 99|SCHED_RR 99|

Contagem letras:

- (PID 11745) a: 303013KB
- (PID 11746) b: 219496KB
- (PID 11747) c: 193862KB
- (PID 11748) d: 283628KB

 <p float="left">
  <img src="https://github.com/schererl/Trabalho3/blob/master/logs/Teste4/ks-img.png" width="800">
  
  <img src="https://github.com/schererl/Trabalho3/blob/master/logs/Teste4/color.png" width="50"/>
 </p>
 
 
 
## Teste 5

O algoritmo other procura balancear as threads por tempo, o que gera uma contagem de letras ainda mais parelha e intercalada para os primeiros 7 processos, enquanto a última thread recebe um tempo muito menor devido a sua prioridade maior.

CPU|MEM(kb)|Threads|th1|th2|th3|th4|th5|th6|th7|th8|
--- | --- | --- |  --- | --- |  --- | --- |  --- | --- |  --- | --- |
2|100000|8|OTHER 0|OTHER 0|OTHER 0|OTHER 0|OTHER 0|OTHER 0|OTHER 0|RR 1|

Contagem letras:

- (PID 12914) a: 9581KB
- (PID 12915) b: 9488KB
- (PID 12916) c: 9483KB
- (PID 12917) d: 9449KB
- (PID 12918) e: 9265KB
- (PID 12919) f: 9614KB
- (PID 12920) g: 8945KB
- (PID 12921) h: 34169KB

 <p float="left">
  <img src="https://github.com/schererl/Trabalho3/blob/master/logs/Teste5/ks-img.png" width="800">
  
  <img src="https://github.com/schererl/Trabalho3/blob/master/logs/Teste5/colors.png" width="50"/>
 </p>
 
 
## Teste 6



CPU|MEM(kb)|Threads|th1|th2|th3|th4|th5|th6|th7|th8|
--- | --- | --- |  --- | --- |  --- | --- |  --- | --- |  --- | --- |
4|100000|8|OTHER 0|FIFO 0|RR 1|FIFO 1|FIFO 1|RR 10|FIFO 10|RR 10|

Contagem letras:

- (PID 13458) a: 2502KB
- (PID 13459) b: 2494KB
- (PID 13460) c: 9982KB
- (PID 13461) d: 9945KB
- (PID 13462) e: 9850KB
- (PID 13463) f: 21021KB
- (PID 13464) g: 23083KB
- (PID 13465) h: 21119KB

 <p float="left">
  <img src="https://github.com/schererl/Trabalho3/blob/master/logs/Teste6/ks-img.png" width="800">
  
  <img src="https://github.com/schererl/Trabalho3/blob/master/logs/Teste6/colors.png" width="50"/>
 </p>



