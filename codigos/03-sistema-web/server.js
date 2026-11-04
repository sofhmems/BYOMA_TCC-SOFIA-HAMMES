const express = require('express');
const session = require('express-session');
const cors = require('cors');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;

app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, 'public')));

// Configure estas variáveis de ambiente em produção (veja .env.example)
app.use(session({
    secret: process.env.SESSION_SECRET || 'dev-apenas-nao-use-em-producao',
    resave: false,
    saveUninitialized: false,
    cookie: { maxAge: 3600000 }
}));

const usuarios = {
    'admin': { senha: process.env.ADMIN_PASSWORD || '123456', nome: 'Administrador' },
    'usuario': { senha: process.env.USER_PASSWORD || 'textil2024', nome: 'Usuário Padrão' }
};

const dadosTecidos = {
    'sintetico': {
        nome: 'Tecido Sintético',
        descricao: 'Produzido a partir de fibras artificiais como poliéster, nylon e acrílico.',
        vantagens: ['Resistente', 'Secagem rápida', 'Durável'],
        desvantagens: ['NÃO biodegradável', 'Libera microplásticos', 'Alto impacto petróleo']
    },
    'natural': {
        nome: 'Tecido Natural',
        descricao: 'Produzido a partir de fibras naturais como algodão, linho, seda e lã.',
        vantagens: ['Biodegradável', 'Confortável', 'Hipoalergênico'],
        desvantagens: ['Maior absorção de água', 'Pode encolher', 'Desgaste mais rápido']
    },
    'misto': {
        nome: 'Tecido Misto',
        descricao: 'Combinação de fibras sintéticas e naturais, buscando o melhor dos dois mundos.',
        vantagens: ['Equilíbrio custo-benefício', 'Resistência mediana', 'Conforto parcial'],
        desvantagens: ['Dificuldade de reciclagem', 'Desempenho mediano em todos os aspectos']
    }
};

function calcularSustentabilidade(tipo, ciclos) {
    const base = {
        'sintetico': { sustentabilidade: 25, densidade: 1.38, desgaste: 0, particulas: 0 },
        'natural': { sustentabilidade: 78, densidade: 1.52, desgaste: 0, particulas: 0 },
        'misto': { sustentabilidade: 52, densidade: 1.45, desgaste: 0, particulas: 0 }
    };

    const dados = JSON.parse(JSON.stringify(base[tipo]));

    const fatorDesgaste = Math.min(ciclos * 0.15, 85);
    dados.desgaste = parseFloat(fatorDesgaste.toFixed(2));

    const particulasBase = tipo === 'sintetico' ? 120 : (tipo === 'natural' ? 15 : 68);
    dados.particulas = parseInt(particulasBase * (1 + ciclos * 0.08));

    const reducao = Math.min(ciclos * 0.5, 40);
    dados.sustentabilidade = Math.max(parseFloat((dados.sustentabilidade - reducao).toFixed(1)), 0);

    dados.densidade_inicial = base[tipo].densidade;
    dados.densidade_final = parseFloat((base[tipo].densidade + (ciclos * 0.002)).toFixed(3));
    dados.diferenca_densidade = parseFloat((dados.densidade_final - dados.densidade_inicial).toFixed(3));

    const anosBase = { 'sintetico': 15, 'natural': 6, 'misto': 10 };
    const particulasPorCiclo = dados.particulas / Math.max(ciclos, 1);
    const reducaoAnos = particulasPorCiclo * 0.05;
    dados.anos_estimados = Math.max(1, parseFloat((anosBase[tipo] - reducaoAnos).toFixed(1)));

    dados.ciclos = ciclos;
    dados.tipo = tipo;

    return dados;
}

function gerarDadosSensor(tipo) {
    const leituras = {
        'sintetico': {
            particulas_liberadas: parseInt(80 + Math.random() * 160),
            densidade_inicial: 1.38,
            densidade_final: parseFloat((1.38 + Math.random() * 0.05).toFixed(3))
        },
        'natural': {
            particulas_liberadas: parseInt(10 + Math.random() * 30),
            densidade_inicial: 1.52,
            densidade_final: parseFloat((1.52 + Math.random() * 0.08).toFixed(3))
        },
        'misto': {
            particulas_liberadas: parseInt(40 + Math.random() * 80),
            densidade_inicial: 1.45,
            densidade_final: parseFloat((1.45 + Math.random() * 0.06).toFixed(3))
        }
    };
    const dados = leituras[tipo];
    dados.diferenca_densidade = parseFloat((dados.densidade_final - dados.densidade_inicial).toFixed(3));

    const anosBase = { 'sintetico': 15, 'natural': 6, 'misto': 10 };
    const particulasPorCiclo = dados.particulas_liberadas / 100;
    dados.anos_estimados = Math.max(1, parseFloat((anosBase[tipo] - particulasPorCiclo * 0.05).toFixed(1)));

    return dados;
}

function autenticar(req, res, next) {
    if (req.session && req.session.usuario) {
        return next();
    }
    res.status(401).json({ erro: 'Não autenticado' });
}

app.post('/api/cadastrar', (req, res) => {
    const { nome, username, password } = req.body;

    if (!nome || !username || !password) {
        return res.status(400).json({ sucesso: false, erro: 'Preencha todos os campos' });
    }

    if (username.length < 3) {
        return res.status(400).json({ sucesso: false, erro: 'Usuário deve ter pelo menos 3 caracteres' });
    }

    if (password.length < 4) {
        return res.status(400).json({ sucesso: false, erro: 'Senha deve ter pelo menos 4 caracteres' });
    }

    if (usuarios[username]) {
        return res.status(400).json({ sucesso: false, erro: 'Usuário já existe' });
    }

    usuarios[username] = { senha: password, nome: nome };
    console.log(`[CADASTRO] Novo usuário: ${username} (${nome})`);
    res.json({ sucesso: true, mensagem: 'Conta criada com sucesso' });
});

app.post('/api/login', (req, res) => {
    const { username, password } = req.body;

    if (usuarios[username] && usuarios[username].senha === password) {
        req.session.usuario = { username, nome: usuarios[username].nome };
        return res.json({ sucesso: true, nome: usuarios[username].nome });
    }

    res.status(401).json({ sucesso: false, erro: 'Usuário ou senha inválidos' });
});

app.post('/api/logout', (req, res) => {
    req.session.destroy();
    res.json({ sucesso: true });
});

app.get('/api/sessao', (req, res) => {
    if (req.session && req.session.usuario) {
        return res.json({ autenticado: true, nome: req.session.usuario.nome });
    }
    res.json({ autenticado: false });
});

app.get('/api/tecidos', (req, res) => {
    res.json(dadosTecidos);
});

app.get('/api/tecido/:tipo', (req, res) => {
    const tipo = req.params.tipo;
    if (dadosTecidos[tipo]) {
        return res.json(dadosTecidos[tipo]);
    }
    res.status(404).json({ erro: 'Tipo de tecido não encontrado' });
});

app.post('/api/analisar', autenticar, (req, res) => {
    const { tipo, ciclos } = req.body;

    if (!dadosTecidos[tipo]) {
        return res.status(400).json({ erro: 'Tipo de tecido inválido' });
    }

    if (!ciclos || ciclos < 1 || ciclos > 1000) {
        return res.status(400).json({ erro: 'Número de ciclos deve estar entre 1 e 1000' });
    }

    const resultado = calcularSustentabilidade(tipo, parseInt(ciclos));
    const sensores = gerarDadosSensor(tipo);

    res.json({
        resultado,
        sensores,
        info: dadosTecidos[tipo]
    });
});

const historicoTestes = [];

app.post('/api/historico/salvar', autenticar, (req, res) => {
    const { tipo, ciclos, sustentabilidade, desgaste, particulas, anos_estimados } = req.body;

    const registro = {
        id: historicoTestes.length + 1,
        usuario: req.session.usuario.username,
        data: new Date().toLocaleString('pt-BR'),
        tipo,
        tipo_nome: dadosTecidos[tipo]?.nome || tipo,
        ciclos,
        sustentabilidade,
        desgaste,
        particulas,
        anos_estimados
    };

    historicoTestes.push(registro);
    res.json({ sucesso: true, id: registro.id });
});

app.get('/api/historico', autenticar, (req, res) => {
    res.json(historicoTestes.filter(t => t.usuario === req.session.usuario.username).reverse());
});

app.get('/api/historico/csv', autenticar, (req, res) => {
    const meusTestes = historicoTestes.filter(t => t.usuario === req.session.usuario.username);

    let csv = 'Data,Tipo de Tecido,Ciclos,Sustentabilidade (%),Desgaste (%),Particulas (ppm),Duracao (anos)\n';
    meusTestes.forEach(t => {
        csv += `${t.data},${t.tipo_nome},${t.ciclos},${t.sustentabilidade},${t.desgaste},${t.particulas},${t.anos_estimados}\n`;
    });

    res.setHeader('Content-Type', 'text/csv; charset=utf-8');
    res.setHeader('Content-Disposition', 'attachment; filename=historico_testes.csv');
    res.send('\uFEFF' + csv);
});

app.post('/api/esp32/dados', (req, res) => {
    const { tipo, particulas_liberadas, densidade_inicial, densidade_final } = req.body;

    console.log(`[ESP32] Dados recebidos - Tipo: ${tipo}`);
    console.log(`[ESP32] Partículas: ${particulas_liberadas} ppm`);
    console.log(`[ESP32] Densidade inicial: ${densidade_inicial} g/cm³ | Densidade final: ${densidade_final} g/cm³`);

    const diferenca = parseFloat((densidade_final - densidade_inicial).toFixed(3));
    const anosBase = { 'sintetico': 15, 'natural': 6, 'misto': 10 };
    const particulasPorCiclo = particulas_liberadas / 100;
    const anos = Math.max(1, parseFloat((anosBase[tipo] - particulasPorCiclo * 0.05).toFixed(1)));

    res.json({
        recebido: true,
        timestamp: new Date().toISOString(),
        diferenca_densidade: diferenca,
        anos_estimados: anos,
        mensagem: `Dados do sensor para ${tipo} registrados com sucesso`
    });
});

app.get('/api/esp32/status', (req, res) => {
    res.json({
        conectado: true,
        ultima_leitura: new Date().toISOString(),
        firmware: "ESP32-SUST-TEXTIL-v2.0",
        sensores: ["Sensor de Partículas GP2Y1010AU0F", "Sensor de Densidade"]
    });
});

app.get('*', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'login.html'));
});

app.listen(PORT, () => {
    console.log(`========================================`);
    console.log(`  SISTEMA DE ANÁLISE DE SUSTENTABILIDADE`);
    console.log(`  TÊXTIL - Servidor rodando na porta ${PORT}`);
    console.log(`  Acesse: http://localhost:${PORT}`);
    console.log(`========================================`);
    console.log(`  Credenciais:`);
    console.log(`    admin / 123456`);
    console.log(`    usuario / textil2024`);
    console.log(`========================================`);
});
