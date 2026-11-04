document.getElementById('form-cadastro').addEventListener('submit', async (e) => {
    e.preventDefault();

    const nome = document.getElementById('nome-completo').value;
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    const confirmar = document.getElementById('confirmar-senha').value;
    const erroEl = document.getElementById('erro-cadastro');
    const sucessoEl = document.getElementById('sucesso-cadastro');
    const loadingEl = document.getElementById('loading-cadastro');
    const btnEl = document.getElementById('btn-cadastrar');

    erroEl.style.display = 'none';
    sucessoEl.style.display = 'none';

    if (password !== confirmar) {
        erroEl.textContent = 'Senhas não conferem';
        erroEl.style.display = 'block';
        return;
    }

    loadingEl.classList.add('active');
    btnEl.disabled = true;

    try {
        const res = await fetch('/api/cadastrar', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ nome, username, password })
        });

        const data = await res.json();

        if (data.sucesso) {
            sucessoEl.style.display = 'block';
            setTimeout(() => {
                window.location.href = 'login.html';
            }, 1500);
        } else {
            erroEl.textContent = data.erro || 'Erro ao cadastrar';
            erroEl.style.display = 'block';
        }
    } catch (err) {
        erroEl.textContent = 'Erro de conexão com o servidor';
        erroEl.style.display = 'block';
    } finally {
        loadingEl.classList.remove('active');
        btnEl.disabled = false;
    }
});
