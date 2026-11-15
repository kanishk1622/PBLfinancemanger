// Goals: savings targets. Progress and "months needed" are calculated by the C++ Goal class.
import { useState } from 'react';
import { CalendarClock, Pencil, Plus, Target, TriangleAlert, Trophy } from 'lucide-react';
import { api } from '../api.js';
import { useData } from '../useData.js';
import { money } from '../format.js';
import { EmptyState, PageHeader, ProgressBar } from '../components/Common.jsx';
import { useToast } from '../components/Toast.jsx';
import Modal from '../components/Modal.jsx';
import { GoalForm, GoalUpdateForm } from '../components/Forms.jsx';

export default function Goals() {
  const { data, error, reload } = useData(() => api('/api/goals'));
  const [adding, setAdding] = useState(false);
  const [editing, setEditing] = useState(null);
  const toast = useToast();

  if (error) return <EmptyState icon={TriangleAlert} title="Could not load goals" text={error} />;
  if (!data) return <p className="loading">Loading…</p>;
  const goals = data.goals;
  const nextId = goals.reduce((max, g) => Math.max(max, g.id), 0) + 1;

  const addGoal = async values => {
    await api('/api/goals/add', values);
    setAdding(false);
    toast(`Goal "${values.name}" added.`);
    await reload();
  };

  const updateGoal = async values => {
    await api('/api/goals/update', { number: editing.number, ...values });
    setEditing(null);
    toast('Goal updated.');
    await reload();
  };

  return (
    <>
      <PageHeader title="Goals" subtitle="What you are saving for">
        <button className="button primary" onClick={() => setAdding(true)}><Plus size={18} /> Add goal</button>
      </PageHeader>

      {goals.length === 0 ? (
        <section className="card"><EmptyState icon={Target} title="No goals yet" text="e.g. a laptop for ₹60,000 by December." /></section>
      ) : (
        <div className="card-grid">
          {goals.map(g => {
            const achieved = g.remaining === 0;
            return (
              <section key={g.number} className={'card goal-card' + (achieved ? ' achieved' : '')}>
                <div className="card-head">
                  <div className="cell-category">
                    <span className="category-icon goal-icon">{achieved ? <Trophy size={19} /> : <Target size={19} />}</span>
                    <div><p className="strong">{g.name}</p><p className="muted small">Goal ID {g.id}</p></div>
                  </div>
                  <button className="icon-button" aria-label={`Update goal ${g.name}`} onClick={() => setEditing(g)}><Pencil size={17} /></button>
                </div>
                <div className="goal-numbers">
                  <p className="big-number">{money(g.savings)}</p>
                  <p className="percent">{g.progress.toFixed(0)}%</p>
                </div>
                <ProgressBar percent={g.progress} color={achieved ? 'var(--good)' : 'var(--accent)'} />
                <p className="muted small">of {money(g.target)}</p>
                <div className="goal-facts">
                  <span><CalendarClock size={15} /> by {g.date}</span>
                  <span className={achieved ? 'good strong' : ''}>
                    {achieved ? 'Goal achieved!'
                      : g.months === -1 ? 'Set a monthly saving to see the time needed'
                      : `${g.months} month(s) at ${money(g.monthly)}/month`}
                  </span>
                </div>
              </section>
            );
          })}
        </div>
      )}

      {adding && (
        <Modal title="Add savings goal" onClose={() => setAdding(false)}>
          <GoalForm suggestedId={nextId} onSubmit={addGoal} onCancel={() => setAdding(false)} />
        </Modal>
      )}
      {editing && (
        <Modal title={`Update goal: ${editing.name}`} onClose={() => setEditing(null)}>
          <GoalUpdateForm goal={editing} onSubmit={updateGoal} onCancel={() => setEditing(null)} />
        </Modal>
      )}
    </>
  );
}
