// Bills go through two C++ data structures, like an operating-system scheduler:
//   1. SCHEDULED: TransactionHeap, a MIN-HEAP ordered by due date (decides WHEN)
//   2. READY:     TransactionQueue, a FIFO circular-array queue (decides the ORDER of payment)
// This page only shows them; the C++ program does the work.
import { useState } from 'react';
import { ArrowDownToLine, CalendarClock, ListOrdered, Play, Plus, Redo2, TriangleAlert } from 'lucide-react';
import { api } from '../api.js';
import { useData } from '../useData.js';
import { dateKey, money, toAppDate, todayIso } from '../format.js';
import { CategoryIcon, EmptyState, PageHeader } from '../components/Common.jsx';
import { useToast } from '../components/Toast.jsx';
import Modal from '../components/Modal.jsx';
import { TransactionForm } from '../components/Forms.jsx';
import { showBudgetToast } from '../components/budgetToast.js';

async function loadBoth() {
  const [scheduled, ready] = await Promise.all([api('/api/scheduled'), api('/api/pending')]);
  return { scheduled: scheduled.scheduled, today: scheduled.today, ready: ready.pending };
}

function BillRow({ bill, badge, action }) {
  return (
    <li className={'queue-item' + (badge?.className === 'next-badge' ? ' next' : '')}>
      <span className="queue-position">{bill.position}</span>
      <CategoryIcon category={bill.category} />
      <div className="tx-main">
        <span className="tx-title">{bill.category} {badge && <span className={'badge ' + badge.className}>{badge.text}</span>}</span>
        <span className="muted">{bill.description ? `${bill.description} · ` : ''}due {bill.date} · {bill.account}</span>
      </div>
      <span className={'tx-amount ' + (bill.type === 'Income' ? 'good' : 'bad')}>
        {bill.type === 'Income' ? '+' : '−'}{money(bill.amount)}
      </span>
      {action}
    </li>
  );
}

export default function Pending() {
  const { data, error, reload } = useData(loadBoth);
  const [adding, setAdding] = useState(false);
  const [releaseDate, setReleaseDate] = useState(todayIso());
  const toast = useToast();

  if (error) return <EmptyState icon={TriangleAlert} title="Could not load bills" text={error} />;
  if (!data) return <p className="loading">Loading…</p>;
  const { scheduled, ready } = data;
  // "Due" means due on or before the release date chosen above (default: today).
  const releaseKey = dateKey(toAppDate(releaseDate));
  const isDue = bill => dateKey(bill.date) <= releaseKey;
  const dueCount = scheduled.filter(isDue).length;

  // Runs an action and turns any error from the server into a toast.
  const run = async action => {
    try { await action(); } catch (err) { toast(err.message, 'error'); }
    await reload();
  };

  const addBill = async values => {
    await api('/api/scheduled/add', values);
    setAdding(false);
    toast(`Bill scheduled for ${values.date}. It changes your balance only when it is paid.`);
    await reload();
  };

  const release = () => run(async () => {
    const date = toAppDate(releaseDate);
    const result = await api('/api/pending/release', { date });
    if (result.released === 0) toast(`No bills are due on or before ${date}.`, 'warn');
    else toast(`${result.released} bill(s) moved to the ready queue, earliest first.`);
    if (result.queueFull) toast('The ready queue is full; the remaining bills stay scheduled.', 'warn');
  });

  // Release ONE bill: the C++ heap removes it from wherever it is (not only the top).
  const releaseOne = bill => run(async () => {
    await api('/api/scheduled/release', { seq: bill.seq, date: toAppDate(releaseDate) });
    toast(`${bill.category} (${money(bill.amount)}, due ${bill.date}) moved to the ready queue.`);
  });

  const payNext = () => run(async () => {
    const result = await api('/api/pending/process', {});
    showBudgetToast(toast, result, `Paid: recorded as transaction ID ${result.id}.`);
  });

  const moveToBack = () => run(async () => {
    await api('/api/pending/move-to-back', {});
    toast('The front bill moved to the back of the queue.');
  });

  return (
    <>
      <PageHeader title="Bills" subtitle="Scheduled by due date (min-heap), then paid in order (FIFO queue)">
        <button className="button primary" onClick={() => setAdding(true)}><Plus size={18} /> Add bill</button>
      </PageHeader>

      <section className="card">
        <div className="card-head">
          <div>
            <h2>Scheduled bills</h2>
            <p className="muted small">Min-heap: the earliest due date is always on top · {scheduled.length} scheduled, {dueCount} due by the release date</p>
          </div>
        </div>
        <div className="release-bar">
          <label className="field">
            <span>Release bills due on or before</span>
            <input type="date" value={releaseDate} onChange={event => setReleaseDate(event.target.value)} aria-label="Release date" />
          </label>
          <button className="button secondary" onClick={release} disabled={scheduled.length === 0}>
            <ArrowDownToLine size={18} /> Release due bills
          </button>
        </div>
        {scheduled.length === 0 ? (
          <EmptyState icon={CalendarClock} title="No scheduled bills" text="Add a bill with its due date; it waits here until it is due." />
        ) : (
          <ol className="queue">
            {scheduled.map(bill => (
              <BillRow key={bill.seq} bill={bill}
                       badge={isDue(bill) ? { text: 'Due', className: 'due-badge' } : { text: 'Upcoming', className: '' }}
                       action={isDue(bill) && (
                         <button className="button secondary small-button" onClick={() => releaseOne(bill)}
                                 aria-label={`Release ${bill.category} due ${bill.date}`}>
                           <ArrowDownToLine size={16} /> Release
                         </button>
                       )} />
            ))}
          </ol>
        )}
      </section>

      <section className="card">
        <div className="card-head">
          <div>
            <h2>Ready to pay</h2>
            <p className="muted small">FIFO queue: the first bill released is the first one paid · {ready.length} waiting</p>
          </div>
          <div className="page-actions">
            <button className="button secondary" onClick={moveToBack} disabled={ready.length < 2} title="If the front bill cannot be paid yet">
              <Redo2 size={18} /> Move front to back
            </button>
            <button className="button primary" onClick={payNext} disabled={ready.length === 0}><Play size={18} /> Pay next</button>
          </div>
        </div>
        {ready.length === 0 ? (
          <EmptyState icon={ListOrdered} title="Nothing ready to pay" text="Release due bills to move them here." />
        ) : (
          <ol className="queue">
            {ready.map(bill => (
              <BillRow key={bill.position} bill={bill} badge={bill.position === 1 ? { text: 'Next', className: 'next-badge' } : null} />
            ))}
          </ol>
        )}
      </section>

      {adding && (
        <Modal title="Add bill" onClose={() => setAdding(false)}>
          <TransactionForm submitLabel="Schedule bill" onSubmit={addBill} onCancel={() => setAdding(false)} />
        </Modal>
      )}
    </>
  );
}
