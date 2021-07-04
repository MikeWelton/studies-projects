package voter;

import main.Candidate;
import main.Party;

import java.util.List;
import java.util.Map;
import java.util.Random;

public abstract class Voter {
	protected String name;
	protected String surname;
	protected int numOfDistrict;
	protected Candidate chosenCandidate;
	
	public Voter(String name, String surname, int numOfDistrict) {
		this.name = name;
		this.surname = surname;
		this.numOfDistrict = numOfDistrict;
	}
	
	
	public int getNumOfDistrict() {
		return this.numOfDistrict;
	}
	
	public void setNumOfDistrict(int numOfDistrict) {
		this.numOfDistrict = numOfDistrict;
	}
	
	/* Głosujący otrzymuje kartę wyborczą i wybiera jednego kandydata. To w jaki
	* sposób dokonuje wyboru zależy od typu wyborcy. */
	public abstract void vote(Map<Party, List<Candidate>> ticket);
	
	public String toString() {
		String ret = "";
		ret += name + " " + surname + " ";
		ret += chosenCandidate.getName() + " " +  chosenCandidate.getSurname();
		return ret;
	}
	
	/* Wyborca podejmuje decyzję, czy podany jako argument kandydat jest lepszy od
	zapisanego w atrybucie jest. Funkcja jest nadpisywana przez wszystkie rodzaje wyborców.
	 */
	protected void decideIfBetter(Candidate candidate) {}
	
	/* Przechodzi po całej karcie wyborczej, czyli po kandydatach wszystkich partii i
	pyta wyborcy, który z nich jest dla niego lepszy.
	 */
	protected void iterateOverTicket(Map<Party, List<Candidate>> ticket) {
		this.chosenCandidate = ticket.entrySet().iterator().next().getValue().get(0);
		/* Przypisujemy bazowo pierwszego kandydata, żeby funkcje porównujące miały do
		 * czego się odnosić. */
		
		for(Map.Entry<Party, List<Candidate>> entry: ticket.entrySet()) {
			List<Candidate> listOfCandidates = entry.getValue();
			for (Candidate candidate: listOfCandidates) {
				decideIfBetter(candidate);
			}
		}
	}
	
	/* Przechodzi po liście wyborczej danej partii i pyta głosującego, który kandydat
	jest lepszy.
	 */
	protected void iterateOverPartyList(List<Candidate> listOfCandidates) {
		this.chosenCandidate = listOfCandidates.get(0);
		/* Przypisujemy bazowo pierwszego kandydata, żeby funkcje porównujące miały do
		* czego się odnosić. */
		
		for(Candidate candidate: listOfCandidates) {
			decideIfBetter(candidate);
		}
	}
	
	protected Candidate chooseRandomly(Candidate candidate1, Candidate candidate2) {
		Random random = new Random();
		int n = random.nextInt(2);
		if(n == 0) {
			return candidate1;
		}
		else {
			return candidate2;
		}
	}
	
	/* Liczy łączną wartość o jaką zmieni się suma ważona kandydata liczona przez wyborcę.
	Wyborcy wszechstronni nadpisują tę funkcję, dla pozostałych typów nic się nie dzieje.
	 */
	public int calculateInfluence(int[] action, Candidate candidate) {
		return 0;
	}
	
	/* Wyborcy wszechstronni nadpisują tę funkcję, bo tylko na nich wpływa kampania.
	 * Dla pozostałych typow wyborców nic się nie dzieje. */
	public void influenceOpinion(int[] action) {}
}
