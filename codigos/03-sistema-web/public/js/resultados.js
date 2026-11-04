const nomesTecidos = {
    'sintetico': 'Tecido Sintético',
    'natural': 'Tecido Natural',
    'misto': 'Tecido Misto'
};

const iconesTecidos = {
    'sintetico': '🧪',
    'natural': '🌿',
    'misto': '🔄'
};

document.addEventListener('DOMContentLoaded', async () => {
    const res = await fetch('/api/sessao');
    const data = await res.json();

    if (!data.autenticado) {
        window.location.href = 'login.html';
        return;
    }

    document.getElementById('nome-usuario').textContent = data.nome;

    const stored = localStorage.getItem('resultadoAnalise');
    if (!stored) {
        window.location.href = 'dashboard.html';
        return;
    }

    const analise = JSON.parse(stored);
    exibirResultados(analise);
});

function exibirResultados(analise) {
    const { resultado, sensores, info } = analise;

    document.getElementById('result-title').textContent =
        `${iconesTecidos[resultado.tipo]} ${nomesTecidos[resultado.tipo]}`;
    document.getElementById('result-subtitle').textContent =
        `Análise após ${resultado.ciclos} ciclos de desgaste`;

    const gauge = document.getElementById('gauge');
    const perc = resultado.sustentabilidade;
    document.getElementById('percentual-sust').textContent = `${perc}%`;

    if (perc >= 60) gauge.className = 'gauge-circle alta';
    else if (perc >= 35) gauge.className = 'gauge-circle media';
    else gauge.className = 'gauge-circle baixa';

    document.getElementById('valor-desgaste').textContent = `${resultado.desgaste}%`;
    document.getElementById('valor-particulas').textContent = resultado.particulas;
    document.getElementById('valor-anos').textContent = resultado.anos_estimados;

    document.getElementById('info-title').textContent = `${info.nome}`;
    document.getElementById('info-descricao').textContent = info.descricao;

    const vantagensEl = document.getElementById('info-vantagens');
    vantagensEl.innerHTML = '';
    info.vantagens.forEach(v => {
        const li = document.createElement('li');
        li.textContent = v;
        vantagensEl.appendChild(li);
    });

    const desvantagensEl = document.getElementById('info-desvantagens');
    desvantagensEl.innerHTML = '';
    info.desvantagens.forEach(d => {
        const li = document.createElement('li');
        li.textContent = d;
        desvantagensEl.appendChild(li);
    });

    const medicoesEl = document.getElementById('lista-medicoes');
    medicoesEl.innerHTML = '';

    const items = [
        { nome: 'Partículas Liberadas (sensor)', valor: `${sensores.particulas_liberadas} ppm` },
        { nome: 'Densidade Inicial', valor: `${sensores.densidade_inicial} g/cm³` },
        { nome: 'Densidade Final', valor: `${sensores.densidade_final} g/cm³` },
        { nome: 'Diferença de Densidade', valor: `${sensores.diferenca_densidade} g/cm³` },
        { nome: 'Duração Estimada', valor: `${sensores.anos_estimados} anos` }
    ];

    items.forEach(item => {
        const div = document.createElement('div');
        div.className = 'medicao-item';
        div.innerHTML = `
            <span class="nome">${item.nome}</span>
            <span class="valor">${item.valor}</span>
        `;
        medicoesEl.appendChild(div);
    });

    document.getElementById('loading-result').classList.remove('active');
    document.getElementById('conteudo-resultados').classList.remove('hidden');

    salvarHistorico(resultado);
}

async function salvarHistorico(resultado) {
    try {
        await fetch('/api/historico/salvar', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                tipo: resultado.tipo,
                ciclos: resultado.ciclos,
                sustentabilidade: resultado.sustentabilidade,
                desgaste: resultado.desgaste,
                particulas: resultado.particulas,
                anos_estimados: resultado.anos_estimados
            })
        });
    } catch (err) {
        console.log('Erro ao salvar histórico');
    }
}

document.getElementById('btn-sair').addEventListener('click', async (e) => {
    e.preventDefault();
    await fetch('/api/logout', { method: 'POST' });
    window.location.href = 'login.html';
});
