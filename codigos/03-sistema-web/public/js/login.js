document.getElementById('form-login').addEventListener('submit', async (e) => {
    e.preventDefault();

    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    const erroEl = document.getElementById('erro-login');
    const loadingEl = document.getElementById('loading-login');
    const btnEl = document.getElementById('btn-entrar');

    erroEl.style.display = 'none';
    loadingEl.classList.add('active');
    btnEl.disabled = true;

    try {
        const res = await fetch('/api/login', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, password })
        });

        const data = await res.json();

        if (data.sucesso) {
            window.location.href = 'dashboard.html';
        } else {
            erroEl.textContent = data.erro || 'Credenciais inválidas';
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
