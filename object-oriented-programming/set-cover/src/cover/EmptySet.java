package cover;

import java.util.ArrayList;
import java.util.TreeSet;

public class EmptySet extends Set {
	public EmptySet() {
		this.elements = new TreeSet<Integer>();
		this.sequences = new ArrayList<Sequence>();
	}
	
	@Override
	public boolean isEmpty() {
		return true;
	}
	
	@Override
	public int sizeOfCommonPart(Set setToCover) {
		return 0;
	}
}
