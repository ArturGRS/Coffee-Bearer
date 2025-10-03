/**
 * @file script.js
 * @description Lida com a interatividade e a comunicação com a API para a interface de gerenciamento da Cafeteira RFID.
 */

// --- FUNÇÕES DE COMUNICAÇÃO COM A API ---

/**
 * Função genérica para realizar chamadas à API e tratar respostas.
 * @param {string} url - O endpoint da API (ex: '/api/status').
 * @param {object} options - As opções para a chamada fetch (método, headers, body, etc.).
 * @returns {Promise<object>} - Os dados da resposta em JSON.
 * @throws {Error} - Lança um erro se a resposta da rede não for bem-sucedida.
 */
async function fetchAPI(url, options = {}) {
    try {
        const response = await fetch(url, options);
        if (!response.ok) {
            const errorData = await response.json().catch(() => ({ message: 'Erro desconhecido na resposta do servidor' }));
            throw new Error(errorData.message || `Erro HTTP: ${response.status}`);
        }
        return await response.json();
    } catch (error) {
        console.error(`Erro na chamada API para ${url}:`, error);
        alert(`Erro na comunicação com o dispositivo: ${error.message}`);
        throw error;
    }
}


// --- FUNÇÕES DE ATUALIZAÇÃO DA INTERFACE ---

/**
 * Busca e atualiza o status do sistema e a lista de usuários.
 */
async function atualizarStatusCompleto() {
    try {
        // Busca o status do sistema
        const data = await fetchAPI('/api/status');

        document.getElementById('total-usuarios').textContent = data.total_usuarios || 0;
        document.getElementById('total-cafes').textContent = data.total_cafes_servidos || 0;
        document.getElementById('cafes-restantes').textContent = `${data.cafes_restantes || 0} / ${data.max_cafes || 0}`;

        const statusDiv = document.getElementById('status-sistema');
        const eventoDiv = document.getElementById('ultimo-evento');

        if (data.sistema_ocupado) {
            statusDiv.className = 'status-sistema status-ocupado';
            statusDiv.querySelector('strong').textContent = 'Sistema Ocupado';
        } else {
            statusDiv.className = 'status-sistema';
            statusDiv.querySelector('strong').textContent = 'Sistema Ativo';
        }
        eventoDiv.textContent = data.ultimo_evento || 'Aguardando eventos...';

        // Aproveita a atualização para buscar a lista de usuários
        await listarUsuarios();

    } catch (error) {
        // O erro já é tratado dentro da fetchAPI
    }
}

/**
 * Busca e renderiza a lista de usuários cadastrados.
 */
async function listarUsuarios() {
    try {
        const data = await fetchAPI('/api/usuarios');
        const listaDiv = document.getElementById('usuarios-lista');
        listaDiv.innerHTML = ""; // Limpa a lista antes de renderizar

        if (!data.usuarios || data.usuarios.length === 0) {
            listaDiv.innerHTML = '<div style="text-align: center; color: #666; padding: 20px;">Nenhum usuário cadastrado</div>';
        } else {
            data.usuarios.forEach(usuario => {
                const userDiv = document.createElement('div');
                userDiv.className = 'usuario-item';
                userDiv.innerHTML = `
                    <div class="usuario-info">
                        <div class="usuario-nome">${usuario.nome}</div>
                        <div class="usuario-uid">${usuario.uid}</div>
                    </div>
                    <div class="usuario-creditos">
                        <strong>${usuario.creditos}</strong> créditos
                    </div>
                    <button class="btn btn-danger" onclick="removerUsuario('${usuario.uid}')">Remover</button>`;
                listaDiv.appendChild(userDiv);
            });
        }
    } catch (error) {
        document.getElementById('usuarios-lista').innerHTML = '<div style="text-align: center; color: #f00; padding: 20px;">Falha ao carregar usuários.</div>';
    }
}


// --- FUNÇÕES DE AÇÕES DO USUÁRIO (EVENTOS) ---

/**
 * Adiciona um novo usuário a partir do formulário.
 * @param {Event} event - O evento de submissão do formulário.
 */
async function adicionarUsuario(event) {
    event.preventDefault();
    const uidInput = document.getElementById('uid');
    const nomeInput = document.getElementById('nome');
    const uid = uidInput.value.trim().toUpperCase();
    const nome = nomeInput.value.trim();

    if (!uid || !nome) {
        alert('Por favor, preencha todos os campos.');
        return;
    }

    try {
        const result = await fetchAPI('/api/usuarios', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ uid, nome }),
        });

        if (result.success) {
            alert('Usuário adicionado com sucesso!');
            uidInput.value = "";
            nomeInput.value = "";
            await atualizarStatusCompleto(); // Atualiza tudo
        }
    } catch (error) {
        // O erro já foi tratado na função fetchAPI
    }
}

/**
 * Remove um usuário com base no UID.
 * @param {string} uid - O UID do usuário a ser removido.
 */
async function removerUsuario(uid) {
    if (!confirm('Tem certeza que deseja remover este usuário?')) {
        return;
    }

    try {
        const result = await fetchAPI('/api/usuarios', {
            method: 'DELETE',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ uid }),
        });

        if (result.success) {
            alert('Usuário removido com sucesso!');
            await atualizarStatusCompleto(); // Atualiza tudo
        }
    } catch (error) {
        // O erro já foi tratado
    }
}

/**
 * Envia um comando para servir um café manualmente.
 */
async function servirCafeManual() {
    if (!confirm('Deseja servir um café manualmente?')) return;
    try {
        const result = await fetchAPI('/api/servir-cafe', { method: 'POST' });
        if (result.success) {
            alert('Comando para servir café enviado!');
            await atualizarStatusCompleto();
        }
    } catch (error) {
        // O erro já foi tratado
    }
}

/**
 * Inicia o processo de limpeza de todos os dados do sistema.
 */
async function confirmarLimpeza() {
    const confirmacao = prompt('ATENÇÃO: Isso apagará TODOS os dados! Para confirmar, digite: LIMPAR TUDO');
    if (confirmacao !== 'LIMPAR TUDO') {
        alert('Operação cancelada.');
        return;
    }

    try {
        const result = await fetchAPI('/api/limpar-dados', { method: 'DELETE' });
        if (result.success) {
            alert('Todos os dados foram apagados!');
            await atualizarStatusCompleto();
        }
    } catch (error) {
        // O erro já foi tratado
    }
}

/**
 * Gera e baixa um arquivo de texto com os dados dos usuários.
 */
async function fazerBackup() {
    try {
        const data = await fetchAPI('/api/backup');
        if (!data.usuarios || data.usuarios.length === 0) {
            alert('Não há dados para fazer backup.');
            return;
        }

        let backupText = `// BACKUP DOS DADOS - ${new Date().toLocaleString()}\r\n\r\n`;
        data.usuarios.forEach(usuario => {
            backupText += `ADD ${usuario.uid} ${usuario.nome}\r\n`;
        });

        const blob = new Blob([backupText], { type: 'text/plain' });
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `backup_cafeteira_${new Date().toISOString().slice(0, 10)}.txt`;
        document.body.appendChild(a);
        a.click();
        window.URL.revokeObjectURL(url);
        document.body.removeChild(a);
        alert('Backup realizado com sucesso!');
    } catch (error) {
        // O erro já foi tratado
    }
}

// --- INICIALIZAÇÃO E EVENT LISTENERS ---

// Garante que o DOM está carregado antes de executar o script
document.addEventListener('DOMContentLoaded', () => {
    // Carrega os dados iniciais assim que a página é carregada
    atualizarStatusCompleto();

    // Configura a atualização automática (polling) a cada 5 segundos
    setInterval(atualizarStatusCompleto, 5000);
});

// Em seu HTML, o botão "Atualizar Status" chama "atualizarStatusCompleto"
// <button class="btn" onclick="atualizarStatusCompleto()">Atualizar Status</button>
// No entanto, como já temos a atualização automática, o botão é opcional,
// mas o deixaremos para atualizações manuais imediatas.
// A função chamada no index.html deve ser a "atualizarStatusCompleto"
// que agora é a principal.

// Para o botão já existente não quebrar:
const atualizarStatus = atualizarStatusCompleto;