let tipoSelecionado = null;

document.addEventListener('DOMContentLoaded', async () => {
    const res = await fetch('/api/sessao');
    const data = await res.json();

    if (!data.autenticado) {
        window.location.href = 'login.html';
        return;
    }

    document.getElementById('nome-usuario').textContent = data.nome;
});

function selecionarTecido(tipo) {
    tipoSelecionado = tipo;

    document.querySelectorAll('.opcao-tecido').forEach(el => {
        el.classList.remove('selecionado');
    });

    document.querySelector(`.opcao-tecido[data-tipo="${tipo}"]`).classList.add('selecionado');
    document.getElementById('btn-analisar').disabled = false;
}

function atualizarCiclos(valor) {
    document.getElementById('valor-ciclos').textContent = valor;
}

async function analisar() {
    if (!tipoSelecionado) {
        alert('Selecione um tipo de tecido');
        return;
    }

    const ciclos = document.getElementById('slider-ciclos').value;
    const btnAnalisar = document.getElementById('btn-analisar');
    btnAnalisar.disabled = true;
    btnAnalisar.textContent = 'Analisando...';

    try {
        const res = await fetch('/api/analisar', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ tipo: tipoSelecionado, ciclos: parseInt(ciclos) })
        });

        if (!res.ok) {
            const err = await res.json();
            alert(err.erro || 'Erro ao realizar análise');
            btnAnalisar.disabled = false;
            btnAnalisar.textContent = 'Confirmar Análise';
            return;
        }

        const data = await res.json();
        localStorage.setItem('resultadoAnalise', JSON.stringify(data));
        window.location.href = 'resultados.html';
    } catch (err) {
        alert('Erro de conexão com o servidor');
        btnAnalisar.disabled = false;
        btnAnalisar.textContent = 'Confirmar Análise';
    }
}

document.getElementById('btn-sair').addEventListener('click', async (e) => {
    e.preventDefault();
    await fetch('/api/logout', { method: 'POST' });
    window.location.href = 'login.html';
});
