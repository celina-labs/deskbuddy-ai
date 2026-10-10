<script setup lang="ts">
useHead({
  title: 'DeskBuddy',
  meta: [{ name: 'description', content: 'Local controls for DeskBuddy.' }],
})
</script>

<template>
  <main class="page-shell">
    <section class="dashboard">
      <header class="topbar">
        <div class="brand">
          <span class="eyes" aria-hidden="true"><span /><span /></span>
          <div>
            <h1>DeskBuddy</h1>
            <p>Local dashboard</p>
          </div>
        </div>
        <span class="demo-badge">Demo</span>
      </header>

      <div class="grid">
        <section class="timer-card" aria-labelledby="timer-heading">
          <div class="card-header">
            <h2 id="timer-heading">Pomodoro</h2>
            <span>25 min</span>
          </div>

          <div class="timer">25:00</div>
          <p class="timer-note">Controlled directly on DeskBuddy.</p>
        </section>

        <section class="status-card" aria-labelledby="status-heading">
          <h2 id="status-heading">Device</h2>
          <dl>
            <div><dt>ESP32-S3</dt><dd>Online</dd></div>
            <div><dt>Wi-Fi</dt><dd class="connected">Connected</dd></div>
            <div><dt>Network</dt><dd>Home Wi-Fi</dd></div>
          </dl>
          <p>Demo data until the live connection is added.</p>
        </section>
      </div>
    </section>
  </main>
</template>

<style>
:root {
  color: #332b24;
  background: #d8ccbc;
  font-family: Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
  font-synthesis: none;
}

* { box-sizing: border-box; }

body {
  margin: 0;
  min-width: 320px;
  min-height: 100vh;
}

.page-shell {
  display: grid;
  min-height: 100vh;
  place-items: center;
  padding: 24px;
  background: #d8ccbc;
}

.dashboard {
  width: min(860px, 100%);
  padding: clamp(22px, 4vw, 38px);
  border: 1px solid #c5b6a3;
  border-radius: 24px;
  background: #f2ece3;
  box-shadow: 0 18px 50px rgba(57, 44, 34, .12);
}

.topbar,
.brand,
.card-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 18px;
}

.brand { justify-content: flex-start; }

.eyes {
  display: inline-flex;
  gap: 5px;
  padding: 11px;
  border-radius: 12px;
  background: #3a2b22;
}

.eyes span {
  width: 7px;
  height: 7px;
  border-radius: 50%;
  background: #f2ece3;
}

h1,
h2,
p { margin-top: 0; }

h1 {
  margin-bottom: 2px;
  font-size: 1.15rem;
  letter-spacing: -.02em;
}

.brand p {
  margin-bottom: 0;
  color: #7e6e5f;
  font-size: .78rem;
}

.demo-badge,
.card-header span {
  padding: 6px 10px;
  border-radius: 999px;
  color: #655646;
  background: #e4dacd;
  font-size: .75rem;
  font-weight: 700;
}

.grid {
  display: grid;
  grid-template-columns: 1.3fr .7fr;
  gap: 16px;
  margin-top: 28px;
}

.timer-card,
.status-card {
  border-radius: 18px;
  padding: 24px;
}

.timer-card {
  color: #f4ede4;
  background: #392b22;
}

.status-card {
  border: 1px solid #d8ccbc;
  background: #e9e0d4;
}

h2 {
  margin-bottom: 0;
  font-size: 1rem;
  letter-spacing: -.01em;
}

.timer {
  margin: 44px 0 20px;
  font-family: Georgia, "Times New Roman", serif;
  font-size: clamp(4rem, 12vw, 6.5rem);
  line-height: .85;
  letter-spacing: -.07em;
}

.timer-note {
  margin-bottom: 0;
  color: #c7b8a8;
  font-size: .82rem;
}

.status-card dl { margin: 22px 0 0; }

.status-card dl div {
  display: flex;
  justify-content: space-between;
  gap: 12px;
  padding: 13px 0;
  border-bottom: 1px solid #d5c8b8;
}

.status-card dt { font-weight: 700; }
.status-card dd { margin: 0; color: #766555; }
.status-card .connected { color: #526149; font-weight: 700; }

.status-card > p {
  margin: 18px 0 0;
  color: #766555;
  font-size: .78rem;
  line-height: 1.5;
}

@media (max-width: 680px) {
  .page-shell { padding: 10px; }
  .dashboard { padding: 20px; border-radius: 18px; }
  .grid { grid-template-columns: 1fr; }
}
</style>
