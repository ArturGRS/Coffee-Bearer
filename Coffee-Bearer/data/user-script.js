document.getElementById('consulta-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const uid = document.getElementById('uid-consulta').value.trim().toUpperCase();
    const resultadoDiv = document.getElementById('resultado-consulta');
    const errorMsg = document.getElementById('error-message');

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
            document.getElementById('uid-consultado').textContent = data.uid;
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