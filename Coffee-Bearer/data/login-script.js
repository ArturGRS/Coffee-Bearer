// Script para a funcionalidade de Login do Administrador
document.getElementById('login-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const password = document.getElementById('password').value;
    const errorMessage = document.getElementById('login-error-message');
    errorMessage.innerText = '';

    const response = await fetch('/login', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ password: password })
    });

    const result = await response.json();
    if (result.success) {
        window.location.href = '/'; // Redireciona para a página principal (admin)
    } else {
        errorMessage.innerText = result.message || 'Senha incorreta.';
    }
});

// Script para a nova funcionalidade de Consulta de Usuário
document.getElementById('consulta-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const uid = document.getElementById('uid-consulta').value.trim().toUpperCase();
    const resultadoDiv = document.getElementById('resultado-consulta');
    const errorMsg = document.getElementById('consulta-error-message');

    resultadoDiv.style.display = 'none';
    errorMsg.innerText = '';

    if (!uid) {
        errorMsg.innerText = 'Por favor, insira um UID.';
        return;
    }

    try {
        const response = await fetch(`/api/usuario?uid=${encodeURIComponent(uid)}`);
        const data = await response.json();

        if (response.ok && data.success) {
            document.getElementById('nome-consultado').textContent = data.nome;
            document.getElementById('creditos-consultado').textContent = data.creditos;
            resultadoDiv.style.display = 'block';
        } else {
            errorMsg.innerText = data.message || 'Usuário não encontrado.';
        }
    } catch (error) {
        console.error('Erro ao consultar API:', error);
        errorMsg.innerText = 'Falha na comunicação com a cafeteira.';
    }
});