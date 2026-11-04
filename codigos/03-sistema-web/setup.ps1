Write-Host "============================================" -ForegroundColor Green
Write-Host "  SISTEMA DE ANALISE DE SUSTENTABILIDADE"
Write-Host "  TEXTIL - Configuracao Automatica v2.0"
Write-Host "============================================" -ForegroundColor Green
Write-Host ""

$nodeInstalled = $false
try {
    $nodeVersion = node --version
    $nodeInstalled = $true
    Write-Host "Node.js detectado: $nodeVersion" -ForegroundColor Cyan
} catch {
    Write-Host "Node.js NAO encontrado!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Para executar este projeto, instale o Node.js:" -ForegroundColor Yellow
    Write-Host "  https://nodejs.org/ (versao LTS recomendada)" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Apos instalar, execute este script novamente." -ForegroundColor Yellow
    exit 1
}

Write-Host "Instalando dependencias..." -ForegroundColor Cyan
npm install

if ($LASTEXITCODE -ne 0) {
    Write-Host "Erro ao instalar dependencias!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Dependencias instaladas com sucesso!" -ForegroundColor Green
Write-Host ""
Write-Host "============================================" -ForegroundColor Green
Write-Host "  PARA INICIAR O SERVIDOR:" -ForegroundColor White
Write-Host "  npm start" -ForegroundColor Yellow
Write-Host ""
Write-Host "  Credenciais de acesso:" -ForegroundColor White
Write-Host "    Usuario: admin   Senha: 123456" -ForegroundColor Cyan
Write-Host "    Usuario: usuario Senha: textil2024" -ForegroundColor Cyan
Write-Host ""
Write-Host "  Acessar: http://localhost:3000" -ForegroundColor Yellow
Write-Host "============================================" -ForegroundColor Green
