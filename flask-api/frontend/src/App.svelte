<script>
  import { onMount } from "svelte";
  import SvelteMarkdown from "svelte-markdown";
  import Pump from "../pics/pump_transparent.png";

  let selectedDate = "";
  let loading = false;
  let error = null;
  let analysis = null;

  // Set default date to today
  onMount(() => {
    const today = new Date();
    selectedDate = today.toISOString().split("T")[0];
  });

  async function analyzeDate() {
    if (!selectedDate) {
      error = "Please select a date";
      return;
    }

    loading = true;
    error = null;
    analysis = null;

    try {
      const response = await fetch(
        `http://localhost:5000/api/analyze/${selectedDate}`
      );
      const data = await response.json();

      if (!response.ok) {
        throw new Error(data.error || `HTTP error! status: ${response.status}`);
      }

      if (data.error) {
        throw new Error(data.error);
      }

      analysis = data;
    } catch (err) {
      error = err.message;
    } finally {
      loading = false;
    }
  }

  function handleKeyPress(event) {
    if (event.key === "Enter") {
      analyzeDate();
    }
  }
</script>

<div class="background-pumps">
    {#each Array(10) as _, i}
      <img src={Pump} alt="" class="pump-bg" />
    {/each}
  </div>
<div class="container">
  <header class="header">
    <h1 class="title">Serbian Media Analysis</h1>
    <p class="subtitle">
      Exposing the parallel realities of regime vs. independent media coverage
    </p>
  </header>

  <section class="date-section">
    <div class="date-input-group">
      <input
        type="date"
        bind:value={selectedDate}
        on:keypress={handleKeyPress}
        class="date-input"
        max={new Date().toISOString().split("T")[0]}
      />
      <button
        on:click={analyzeDate}
        disabled={loading || !selectedDate}
        class="analyze-btn"
      >
        {loading ? "Analyzing..." : "Analyze Coverage"}
      </button>
    </div>
  </section>

  {#if loading}
    <div class="loading">
      <div class="spinner"></div>
      <span>Analyzing media coverage with AI...</span>
    </div>
  {/if}

  {#if error}
    <div class="error">
      <strong>Error:</strong>
      {error}
    </div>
  {/if}

  {#if analysis}
    <div class="analysis-result">
      <div class="analysis-header">
        <div class="analysis-date">
          Analysis for {analysis.date}
        </div>
        <div class="stats">
          <div class="stat-item stat-total">
            Total: {analysis.statistics.total_articles}
          </div>
          <div class="stat-item stat-regime">
            Regime: {analysis.statistics.regime_articles}
          </div>
          <div class="stat-item stat-independent">
            Independent: {analysis.statistics.independent_articles}
          </div>
        </div>
      </div>

      <div class="analysis-content">
        <SvelteMarkdown source={analysis.analysis} />
      </div>
    </div>
  {/if}
</div>
