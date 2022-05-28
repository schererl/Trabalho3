# Trabalho3

Victor Putrich & Lucca Dornelles

# Sobre

# Funcionamento

O programa simula o processamento consecutivo e intercalado de vários processos utilizando multiplas threads que escrevem em um buffer. A ordem e tempo reservado a cada thread é dependente do scheduler utilizado. 

### Mutex
Utilizamos dois objetos mutex para coordenar o funcionamento das threads. 
**simLock** trava qualquer thread que tente rodar antes que todas sejam inicializadas, fazendo com que todas threads começem simultaneamente. 
**lock** é utilizado dentro das threads para impedir que uma thread rode enquanto outra esteja na zona crítica (que é a interidade da thread após sua inicialização). Enquanto uma thread está escrevendo no buffer todas as outras esperam e só após o processo de escrita acabar que o scheduler pode trocar de thread. 

### Output
O output retornado pelo processo é uma lista de letras com letras duplicadas removidas (e.g. aaba -> aba), que representa a ordem de execução das threads. 
